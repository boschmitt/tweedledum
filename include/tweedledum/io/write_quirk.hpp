/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace tweedledum {

template<class Iterator, class MapFn, class JoinFn>
auto map_and_join(Iterator begin, Iterator end, MapFn&& map_fn, JoinFn&& join_fn)
{
	return std::accumulate(begin + 1, end, map_fn(*begin),
	                       [&](auto const& a, auto const& v) { return join_fn(a, map_fn(v)); });
}

/*! \brief Writes network in Quirk format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required gate functions:**
 * - `kind`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_node`
 *
 * \param network Network
 * \param os Output stream
 */
template<class Network>
void write_quirk_encoded_json(Network const& network, std::ostream& os)
{
	if (network.num_gates() == 0)
		return;

	std::vector<std::vector<std::string>> cols;

	const auto add_empty_column = [&]() { cols.emplace_back(network.num_qubits(), "1"); };

	const auto add_gate = [&](uint32_t row, std::string const& gate) {
		if (cols.back()[row] != "1")
			add_empty_column();
		cols.back()[row] = gate;
	};

	const auto add_controlled_gate = [&](uint32_t control, uint32_t target,
	                                     std::string const& gate) {
		/* add new column, if current one has gates */
		if (std::find_if(cols.back().begin(), cols.back().end(),
		                 [](auto const& s) { return s != "1"; })
		    != cols.back().end())
			add_empty_column();

		cols.back()[control] = "•";
		cols.back()[target] = gate;
		add_empty_column();
	};

	add_empty_column();
	network.foreach_node([&](auto const& n) {
		auto const& g = n.gate;
		switch (g.kind()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;

		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			return true;

		case gate_kinds_t::hadamard:
			g.foreach_target([&](auto q) { add_gate(q, "H"); });
			break;

		case gate_kinds_t::pauli_x:
			g.foreach_target([&](auto q) { add_gate(q, "X"); });
			break;

		case gate_kinds_t::pauli_z:
			g.foreach_target([&](auto q) { add_gate(q, "Z"); });
			break;

		case gate_kinds_t::phase:
			g.foreach_target([&](auto q) { add_gate(q, "Z^%C2%BD"); });
			break;

		case gate_kinds_t::phase_dagger:
			g.foreach_target([&](auto q) { add_gate(q, "Z^-%C2%BD"); });
			break;

		case gate_kinds_t::t:
			g.foreach_target([&](auto q) { add_gate(q, "Z^%C2%BC"); });
			break;

		case gate_kinds_t::t_dagger:
			g.foreach_target([&](auto q) { add_gate(q, "Z^-%C2%BC"); });
			break;

		case gate_kinds_t::cx:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) { add_controlled_gate(qc, qt, "X"); });
			});
			break;

		case gate_kinds_t::mcx: {
			std::vector<uint32_t> controls, targets;
			g.foreach_control([&](auto q) { controls.push_back(q); });
			g.foreach_target([&](auto q) { targets.push_back(q); });
			switch (controls.size()) {
			default:
				std::cerr << "[w] unsupported control size\n";
				return true;
			case 0u:
				for (auto q : targets)
					add_gate(q, "X");
				break;
			case 1u:
				for (auto q : targets)
					add_controlled_gate(controls[0], q, "X");
				break;
			}
		} break;
		}

		return true;
	});

	const auto join_with_comma = [](std::vector<std::string> const& v) {
		return "["
		       + map_and_join(v.begin(), v.end(),
		                      [](auto const& s) { return s == "1" ? "1" : "\"" + s + "\""; },
		                      [](auto const& a, auto const& b) { return a + "," + b; })
		       + "]";
	};

	/*const auto join_with_comma = [](std::vector<std::string> const& v) {
	        return "["
	               + std::accumulate(v.begin() + 1, v.end(),
	                                 "\"" + v.front() + "\"",
	                                 [](auto const& a, auto const& s) {
	                                         return a + ",\"" + s + "\"";
	                                 })
	               + "]";
	};*/

	os << "\"cols\":["
	   << map_and_join(cols.begin(), cols.end(),
	                   [&](const auto& v) { return join_with_comma(v); },
	                   [&](auto const& a, auto const& b) { return a + "," + b; })
	   << "]\n";
}

/*! \brief Writes network in Quirk format into a file
 *
 * **Required gate functions:**
 * - `kind`
 * - `num_controls`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_node`
 *
 * \param network Network
 * \param filename Filename
 */
template<class Network>
void write_quirk_encoded_json(Network const& network, std::string const& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_quirk_encoded_json(network, os);
}

} // namespace tweedledum
