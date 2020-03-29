/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "../networks/wire_id.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace tweedledum {
namespace detail {

class string_builder {
public:
	string_builder(std::vector<wire_id> const& io)
	    : max_num_glyphs_(0u)
	    , wires_(io)
	    , occupancy_(io.size(), 0)
	    , num_glyphs_(io.size(), 0)
	    , lines_(3 * io.size())
	{
		uint32_t id_size = 0u;
		for (wire_id id : io) {
			id_size = std::max(id_size, num_digits(id));
		}
		for (uint32_t i = 0u; i < wires_.size(); ++i) {
			wire_id id = wires_.at(wires_.size() - (i + 1));
			lines_[(3 * i)] += fmt::format("{:>{}}", "", 7u + id_size);
			if (id.is_qubit()) {
				lines_[(3 * i) + 1] += fmt::format("w{:>{}} : ───", id, id_size);
			} else {
				lines_[(3 * i) + 1] += fmt::format("w{:>{}} : ═══", id, id_size);
			}
			lines_[(3 * i) + 2] += fmt::format("{:>{}}", "", 7u + id_size);
			num_glyphs_.at(i) = 7u + id_size;
		}
		max_num_glyphs_ = 7u + id_size;
	}

	void add_op(std::string_view gate, wire_id target)
	{
		uint32_t line = wires_.size() - (target + 1);
		uint32_t gate_len = utf8_strlen(gate);
		if (occupancy_[line] != 0) {
			finish_column();
		}
		occupancy_[line] = 1;
		lines_[(3 * line)] += fmt::format("┌{:─>{}}┐", "", 2u + gate_len);
		lines_[(3 * line) + 1] += fmt::format("┤ {} ├", gate);
		lines_[(3 * line) + 2] += fmt::format("└{:─>{}}┘", "", 2u + gate_len);
		num_glyphs_.at(line) += 4u + gate_len;
		max_num_glyphs_ = std::max(max_num_glyphs_, num_glyphs_.at(line));
	}

	void add_op(std::string_view gate, wire_id control, wire_id target)
	{
		uint32_t c_line = wires_.size() - (control + 1);
		uint32_t t_line = wires_.size() - (target + 1);
		uint32_t gate_len = utf8_strlen(gate);

		uint32_t const min = std::min(c_line, t_line);
		uint32_t const max = std::max(c_line, t_line);
		if (does_need_new_column(min, max)) {
			finish_column();
		}
		occupancy_[c_line] = 1;
		occupancy_[t_line] = 1;

		lines_[(3 * c_line)] += fmt::format("  {}{:^{}} ", c_line < t_line ? " " : "│", "",
		                                    gate_len);
		lines_[(3 * c_line) + 1] += fmt::format("──{}{:─^{}}─",
		                                        control.is_complemented() ? "◯" : "●", "",
		                                        gate_len);
		lines_[(3 * c_line) + 2] += fmt::format("  {}{:^{}} ", c_line < t_line ? "│" : " ",
		                                        "", gate_len);

		lines_[(3 * t_line)] += fmt::format("┌─{}{:─^{}}┐", c_line < t_line ? "┴" : "─", "",
		                                    gate_len);
		lines_[(3 * t_line) + 1] += fmt::format("┤ {} ├", gate);
		lines_[(3 * t_line) + 2] += fmt::format("└─{}{:─^{}}┘", c_line < t_line ? "─" : "┬",
		                                        "", gate_len);

		for (auto i = min + 1; i < max; ++i) {
			occupancy_[i] = 1;
			lines_[(3 * i)] += fmt::format("  │{:^{}} ", "", gate_len);
			if (wires_[wires_.size() - (i + 1)].is_qubit()) {
				lines_[(3 * i) + 1] += fmt::format("──┼{:─^{}}─", "", gate_len);
			} else {
				lines_[(3 * i) + 1] += fmt::format("══╪{:═^{}}═", "", gate_len);
			}
			lines_[(3 * i) + 2] += fmt::format("  │{:^{}} ", "", gate_len);
			num_glyphs_.at(i) += 4u + gate_len;
		}
		num_glyphs_.at(c_line) += 4u + gate_len;
		num_glyphs_.at(t_line) += 4u + gate_len;
		max_num_glyphs_ = std::max(max_num_glyphs_, num_glyphs_.at(t_line));
	}

	void add_swap(wire_id q0, wire_id q1)
	{
		uint32_t q0_line = wires_.size() - (q0 + 1);
		uint32_t q1_line = wires_.size() - (q1 + 1);
		uint32_t const min = std::min(q0_line, q1_line);
		uint32_t const max = std::max(q0_line, q1_line);
		if (does_need_new_column(min, max)) {
			finish_column();
		}
		occupancy_[q0_line] = 1;
		occupancy_[q1_line] = 1;

		lines_[(3 * q0_line)] += q0_line < q1_line ? "     " : "  │  ";
		lines_[(3 * q0_line) + 1] += "──╳──";
		lines_[(3 * q0_line) + 2] += q0_line < q1_line ? "  │  " : "     ";

		lines_[(3 * q1_line)] += q0_line < q1_line ? "  │  " : "     ";
		lines_[(3 * q1_line) + 1] += "──╳──";
		lines_[(3 * q1_line) + 2] += q0_line < q1_line ? "     " : "  │  ";

		for (auto i = min + 1; i < max; ++i) {
			occupancy_[i] = 1;
			lines_[(3 * i)] += "  │  ";
			if (wires_[wires_.size() - (i + 1)].is_qubit()) {
				lines_[(3 * i) + 1] += "──┼──";
			} else {
				lines_[(3 * i) + 1] += "══╪══";
			}
			lines_[(3 * i) + 2] += "  │  ";
			num_glyphs_.at(i) += 5u;
		}
		num_glyphs_.at(q0_line) += 5u;
		num_glyphs_.at(q1_line) += 5u;
		max_num_glyphs_ = std::max(max_num_glyphs_, num_glyphs_.at(q0_line));
	}

	void add_op(std::string_view gate, std::vector<wire_id> const& cs,
	            std::vector<wire_id> const& ts)
	{
		uint32_t gate_len = utf8_strlen(gate);
		std::vector<uint32_t> c_lines;
		std::vector<uint32_t> t_lines;
		std::transform(cs.begin(), cs.end(), std::back_inserter(c_lines),
		               [&](wire_id id) { return wires_.size() - (id + 1); });
		std::transform(ts.begin(), ts.end(), std::back_inserter(t_lines),
		               [&](wire_id id) { return wires_.size() - (id + 1); });

		const auto [c_min, c_max] = std::minmax_element(c_lines.begin(), c_lines.end());
		const auto [t_min, t_max] = std::minmax_element(t_lines.begin(), t_lines.end());
		const uint32_t min = std::min(*c_min, *t_min);
		const uint32_t max = std::max(*c_max, *t_max);
		if (does_need_new_column(min, max)) {
			finish_column();
		}

		for (uint32_t i = 0; i < c_lines.size(); ++i) {
			uint32_t c_line = c_lines.at(i);
			occupancy_[c_line] = 1;
			lines_[(3 * c_line)] += fmt::format("  {}{:^{}} ",
			                                    c_line == min ? " " : "│", "", gate_len);
			lines_[(3 * c_line) + 1] += fmt::format("──{}{:─^{}}─",
			                                        cs[i].is_complemented() ? "◯" : "●",
			                                        "", gate_len);
			lines_[(3 * c_line) + 2] += fmt::format("  {}{:^{}} ",
			                                        c_line == max ? " " : "│", "",
			                                        gate_len);
			num_glyphs_.at(c_line) += 4u + gate_len;
		}
		for (uint32_t i = 0; i < t_lines.size(); ++i) {
			uint32_t t_line = t_lines.at(i);
			occupancy_[t_line] = 1;
			lines_[(3 * t_line)] += fmt::format("┌─{}{:─^{}}┐",
			                                    t_line == min ? "─" : "┴", "",
			                                    gate_len);
			lines_[(3 * t_line) + 1] += fmt::format("┤ {} ├", gate);
			lines_[(3 * t_line) + 2] += fmt::format("└─{}{:─^{}}┘",
			                                        t_line == max ? "─" : "┬", "",
			                                        gate_len);
			num_glyphs_.at(t_line) += 4u + gate_len;
		}

		for (auto i = min + 1; i < max; ++i) {
			if (occupancy_[i] == 1) {
				continue;
			}
			occupancy_[i] = 1;
			lines_[(3 * i)] += fmt::format("  │{:^{}} ", "", gate_len);
			if (wires_[wires_.size() - (i + 1)].is_qubit()) {
				lines_[(3 * i) + 1] += fmt::format("──┼{:─^{}}─", "", gate_len);
			} else {
				lines_[(3 * i) + 1] += fmt::format("══╪{:═^{}}═", "", gate_len);
			}
			lines_[(3 * i) + 2] += fmt::format("  │{:^{}} ", "", gate_len);
			num_glyphs_.at(i) += 4u + gate_len;
		}
		max_num_glyphs_ = std::max(max_num_glyphs_, num_glyphs_.at(t_lines[0]));
	}

	std::string str()
	{
		finish_column();
		std::string result;
		for (auto const& line : lines_) {
			result += line + "\n";
		}
		return result;
	}

private:
	uint32_t utf8_strlen(std::string_view str)
	{
		uint32_t num_glyphs = 0u;
		uint32_t end = str.length();
		for (uint32_t i = 0; i < end; i++, num_glyphs++) {
			uint8_t c = static_cast<uint8_t>(str.at(i));
			if (c >= 0 && c <= 127) {
				continue;
			} else if ((c & 0xE0) == 0xC0) {
				i += 1;
			} else if ((c & 0xF0) == 0xE0) {
				i += 2;
			} else if ((c & 0xF8) == 0xF0) {
				i += 3;
			} else {
				return 0u;
			}
		}
		return num_glyphs;
	}

	uint32_t num_digits(uint32_t value) const
	{
		return (value < 10 ? 1 :
		       (value < 100 ? 2 :
		       (value < 1000 ? 3 :
		       (value < 10000 ? 4 :
		       (value < 100000 ? 5 :
		       (value < 1000000 ? 6 :
		       (value < 10000000 ? 7 :
		       (value < 100000000 ? 8 :
		       (value < 1000000000 ? 9 : 10)))))))));  
	}

	void finish_column()
	{
		for (auto i = 0u; i < occupancy_.size(); ++i) {
			assert(num_glyphs_.at(i) <= max_num_glyphs_);
			uint32_t pad = max_num_glyphs_ - num_glyphs_.at(i);
			if (pad) {
				lines_[(3 * i)] += fmt::format("{:>{}}", "", pad);
				if (wires_[wires_.size() - (i + 1)].is_qubit()) {
					lines_[(3 * i) + 1] += fmt::format("{:─>{}}", "", pad);
				} else {
					lines_[(3 * i) + 1] += fmt::format("{:═>{}}", "", pad);
				}
				lines_[(3 * i) + 2] += fmt::format("{:>{}}", "", pad);
			}
			num_glyphs_.at(i) = max_num_glyphs_;
			occupancy_[i] = 0;
		}
	}

	bool does_need_new_column(uint32_t from, uint32_t to)
	{
		for (uint32_t i = from; i <= to; ++i) {
			if (occupancy_[i] != 0) {
				return true;
			}
		}
		return false;
	}

private:
	uint32_t max_num_glyphs_;
	std::vector<wire_id> wires_;
	std::vector<uint8_t> occupancy_;
	std::vector<uint32_t> num_glyphs_;
	std::vector<std::string> lines_;
};

template<typename Network, typename Builder>
auto to_utf8_str(Network const& network, Builder builder)
{
	using op_type = typename Network::op_type;
	network.foreach_op([&](op_type const& op) {
		std::vector<wire_id> cs;
		std::vector<wire_id> ts;
		switch (op.id()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			break;

		case gate_ids::h:
			builder.add_op("H", op.target());
			break;
		
		case gate_ids::x:
			builder.add_op("X", op.target());
			break;

		case gate_ids::y:
			builder.add_op("Y", op.target());
			break;

		case gate_ids::z:
			builder.add_op("Z", op.target());
			break;

		case gate_ids::s:
			builder.add_op("S", op.target());
			break;

		case gate_ids::sdg:
			builder.add_op("S†", op.target());
			break;

		case gate_ids::t:
			builder.add_op("T", op.target());
			break;

		case gate_ids::tdg:
			builder.add_op("T†", op.target());
			break;

		case gate_ids::r1:
			builder.add_op("R1", op.target());
			return;

		case gate_ids::rx:
			builder.add_op("Rx", op.target());
			return;

		case gate_ids::ry:
			builder.add_op("Ry", op.target());
			return;

		case gate_ids::rz:
			builder.add_op("Rz", op.target());
			return;

		case gate_ids::u3:
			builder.add_op("U3", op.target());
			return;
		
		case gate_ids::cx:
			builder.add_op("X", op.control(), op.target());
			return;

		case gate_ids::cy:
			builder.add_op("Y", op.control(), op.target());
			return;

		case gate_ids::cz:
			builder.add_op("Z", op.control(), op.target());
			return;

		case gate_ids::swap:
			builder.add_swap(op.target(0u), op.target(1u));
			return;

		case gate_ids::crx:
			builder.add_op("Rx", op.control(), op.target());
			return;

		case gate_ids::cry:
			builder.add_op("Ry", op.control(), op.target());
			return;

		case gate_ids::crz:
			builder.add_op("Rz", op.control(), op.target());
			return;

		case gate_ids::ncx:
			op.foreach_control([&](wire_id c) { cs.push_back(c); });
			op.foreach_target([&](wire_id t) { ts.push_back(t); });
			builder.add_op("X", cs, ts);
			return;

		case gate_ids::ncy:
			op.foreach_control([&](wire_id c) { cs.push_back(c); });
			op.foreach_target([&](wire_id t) { ts.push_back(t); });
			builder.add_op("Y", cs, ts);
			return;

		case gate_ids::ncz:
			op.foreach_control([&](wire_id c) { cs.push_back(c); });
			op.foreach_target([&](wire_id t) { ts.push_back(t); });
			builder.add_op("Z", cs, ts);
			return;

		case gate_ids::ncrx:
			op.foreach_control([&](wire_id c) { cs.push_back(c); });
			op.foreach_target([&](wire_id t) { ts.push_back(t); });
			builder.add_op("Rx", cs, ts);
			return;

		case gate_ids::ncry:
			op.foreach_control([&](wire_id c) { cs.push_back(c); });
			op.foreach_target([&](wire_id t) { ts.push_back(t); });
			builder.add_op("Ry", cs, ts);
			return;

		case gate_ids::ncrz:
			op.foreach_control([&](wire_id c) { cs.push_back(c); });
			op.foreach_target([&](wire_id t) { ts.push_back(t); });
			builder.add_op("Rz", cs, ts);
			return;
		}
	});
	return builder.str();
}

} // namespace detail

/*! \brief Writes a network in Unicode format format into a output stream
 *
 * **Required gate functions:**
 * - `operation`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `foreach_gate`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param fancy (default: true)
 * \param os Output stream (default: std::cout)
 */
template<typename Network>
void write_utf8(Network const& network, std::ostream& os = std::cout)
{
	if (network.num_operations() == 0) {
		return;
	}
	std::string utf8_str;
	std::vector<wire_id> qubits;
	network.foreach_wire([&](wire_id id) {
		qubits.emplace_back(id);
	});
	detail::string_builder builder(qubits);
	utf8_str = detail::to_utf8_str(network, builder);
	os << utf8_str;
}

} // namespace tweedledum
