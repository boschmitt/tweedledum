/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_set.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace tweedledum {
namespace detail {

class fancy_string_builder {
public:
	fancy_string_builder(uint32_t num_qubits)
	    : occupancy_(num_qubits, 0)
	    , lines_(3 * num_qubits, "     ")
	{
		for (auto i = 0u; i < num_qubits; ++i) {
			lines_[(3 * i) + 1].replace(0, 5, "|0>──");
		}
	}

	void add_gate(std::string const& op, qubit_id target)
	{
		if (occupancy_[target] != 0) {
			new_column();
		}
		occupancy_[target] = 1;

		lines_[(3 * target)] += "┌───┐";
		lines_[(3 * target) + 1] += "┤ " + op + " ├";
		lines_[(3 * target) + 2] += "└───┘";
	}

	void add_gate(std::string const& op, qubit_id control, qubit_id target)
	{
		if (!is_last_column_empty()) {
			new_column();
		}
		occupancy_[control] = 1;
		occupancy_[target] = 1;

		lines_[(3 * control)] += control < target ? "     " : "  │  ";
		lines_[(3 * control) + 1] += control.is_complemented() ? "──◯──" : "──●──";
		lines_[(3 * control) + 2] += control < target ? "  │  " : "     ";

		lines_[(3 * target)] += control < target ? "┌─┴─┐" : "┌───┐";
		lines_[(3 * target) + 1] += "┤ " + op + " ├";
		lines_[(3 * target) + 2] += control < target ? "└───┘" : "└─┬─┘";

		auto const min = std::min(control.index(), target.index());
		auto const max = std::max(control.index(), target.index());
		for (auto i = min + 1; i < max; ++i) {
			occupancy_[i] = 1;
			lines_[(3 * i)] += "  │  ";
			lines_[(3 * i) + 1] += "──┼──";
			lines_[(3 * i) + 2] += "  │  ";
		}
		new_column();
	}

	void add_gate(std::string const& op, std::vector<qubit_id> controls,
	              std::vector<qubit_id> targets)
	{
		if (!is_last_column_empty()) {
			new_column();
		}

		const auto [min_target, max_target] = std::minmax_element(targets.begin(),
		                                                          targets.end());
		const auto [min_control, max_control] = std::minmax_element(controls.begin(),
		                                                            controls.end());
		const auto min = std::min(*min_control, *min_target);
		const auto max = std::max(*max_control, *max_target);

		for (auto control : controls) {
			occupancy_[control] = 1;
			lines_[(3 * control)] += control == min ? "     " : "  │  ";
			lines_[(3 * control) + 1] += control.is_complemented() ? "──◯──" : "──●──";
			lines_[(3 * control) + 2] += control == max ? "     " : "  │  ";
		}
		for (auto target : targets) {
			occupancy_[target] = 1;
			lines_[(3 * target)] += target == min ? "┌───┐" : "┌─┴─┐";
			lines_[(3 * target) + 1] += "┤ " + op + " ├";
			lines_[(3 * target) + 2] += target == max ? "└───┘" : "└─┬─┘";
		}

		for (auto i = min.index() + 1; i < max.index(); ++i) {
			if (occupancy_[i] == 1) {
				continue;
			}
			occupancy_[i] = 1;
			lines_[(3 * i)] += "  │  ";
			lines_[(3 * i) + 1] += "──┼──";
			lines_[(3 * i) + 2] += "  │  ";
		}
		new_column();
	}

	std::string str()
	{
		std::string result;
		for (auto const& line : lines_) {
			result += line + "\n";
		}
		return result;
	}

private:
	void new_column()
	{
		for (auto i = 0u; i < occupancy_.size(); ++i) {
			if (occupancy_[i] == 0) {
				lines_[(3 * i)] += "     ";
				lines_[(3 * i) + 1] += "─────";
				lines_[(3 * i) + 2] += "     ";
			}
			occupancy_[i] = 0;
		}
	}

	bool is_last_column_empty()
	{
		for (auto element : occupancy_) {
			if (element == 1) {
				return false;
			}
		}
		return true;
	}

private:
	std::vector<uint8_t> occupancy_;
	std::vector<std::string> lines_;
};

class string_builder {
public:
	string_builder(uint32_t num_qubits)
	    : lines_(num_qubits, "―")
	{ }

	void add_gate(std::string const& op, qubit_id target)
	{
		lines_[target] += op == "X" ? "⊕" : op;
		new_column();
	}

	void add_gate(std::string const& op, qubit_id control, qubit_id target)
	{
		if (control.is_complemented()) {
			lines_[control] += "○";
		} else {
			lines_[control] += "●";
		}
		lines_[target] += op == "X" ? "⊕" : op;
		new_column();
	}

	void add_gate(std::string const& op, std::vector<qubit_id> controls,
	              std::vector<qubit_id> targets)
	{
		for (auto qid : controls) {
			if (qid.is_complemented()) {
				lines_[qid] += "○";
			} else {
				lines_[qid] += "●";
			}
		}
		for (auto qid : targets) {
			lines_[qid] += op == "X" ? "⊕" : op;
		}
		new_column();
	}

	std::string str()
	{
		std::string result;
		for (auto const& line : lines_) {
			result += line + "\n";
		}
		return result;
	}

private:
	void new_column()
	{
		for (auto& line : lines_) {
			if (line.size() % 2 == 0) {
				line += "―";
			} else {
				line += "――";
			}
		}
	}

private:
	std::vector<std::string> lines_;
};

// Maybe use it polymorphism instead of templates
template<typename Network, typename Builder>
auto to_unicode_str(Network const& network, Builder builder)
{
	network.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;

		case gate_set::hadamard:
			gate.foreach_target([&](auto qid) { builder.add_gate("H", qid); });
			break;

		case gate_set::pauli_x:
			gate.foreach_target([&](auto qid) { builder.add_gate("X", qid); });
			break;

		case gate_set::pauli_y:
			gate.foreach_target([&](auto qid) { builder.add_gate("Y", qid); });
			break;

		case gate_set::pauli_z:
			gate.foreach_target([&](auto qid) { builder.add_gate("Z", qid); });
			break;

		case gate_set::rotation_x:
			gate.foreach_target([&](auto qid) { builder.add_gate("x", qid); });
			break;

		case gate_set::rotation_y:
			gate.foreach_target([&](auto qid) { builder.add_gate("y", qid); });
			break;

		case gate_set::rotation_z:
			gate.foreach_target([&](auto qid) { builder.add_gate("z", qid); });
			break;

		case gate_set::phase:
			gate.foreach_target([&](auto qid) { builder.add_gate("S", qid); });
			break;

		case gate_set::phase_dagger:
			gate.foreach_target([&](auto qid) { builder.add_gate("Ƨ", qid); });
			break;

		case gate_set::t:
			gate.foreach_target([&](auto qid) { builder.add_gate("T", qid); });
			break;

		case gate_set::t_dagger:
			gate.foreach_target([&](auto qid) { builder.add_gate("⊥", qid); });
			break;

		case gate_set::cx:
			gate.foreach_control([&](auto qid_control) {
				gate.foreach_target([&](auto qid_target) {
					builder.add_gate("X", qid_control, qid_target);
				});
			});
			break;

		case gate_set::mcx: {
			std::vector<qubit_id> controls;
			std::vector<qubit_id> targets;
			gate.foreach_control([&](auto control) { controls.push_back(control); });
			gate.foreach_target([&](auto target) { targets.push_back(target); });
			builder.add_gate("X", controls, targets);
		} break;

		case gate_set::cz:
			gate.foreach_control([&](auto qid_control) {
				gate.foreach_target([&](auto qid_target) {
					builder.add_gate("Z", qid_control, qid_target);
				});
			});
			break;

		case gate_set::mcz: {
			std::vector<qubit_id> controls;
			std::vector<qubit_id> targets;
			gate.foreach_control([&](auto control) { controls.push_back(control); });
			gate.foreach_target([&](auto target) { targets.push_back(target); });
			builder.add_gate("Z", controls, targets);
		} break;
		}
		return true;
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
 * - `foreach_cgate`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param fancy (default: true)
 * \param os Output stream (default: std::cout)
 */
template<typename Network>
void write_unicode(Network const& network, bool fancy = true, std::ostream& os = std::cout)
{
	if (network.num_gates() == 0) {
		return;
	}

	std::string unicode_str;
	if (fancy) {
		detail::fancy_string_builder builder(network.num_qubits());
		unicode_str = detail::to_unicode_str(network, builder);
	} else {
		detail::string_builder builder(network.num_qubits());
		unicode_str = detail::to_unicode_str(network, builder);
	}
	os << unicode_str;
}

/*! \brief Writes a network in Unicode format format into a file
 *
 * **Required gate functions:**
 * - `operation`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param fancy (default: true)
 * \param filename Filename
 */
template<typename Network>
void write_unicode(Network const& network, std::string const& filename, bool fancy = true)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_unicode(network, os);
}

} // namespace tweedledum
