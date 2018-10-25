/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace tweedledum {

/*! \brief Prints a circuit in unicode format.
 *
 * \param network A quantum circuit
 * \return A string representing the circuit using Unicode characters
 */
template<class Network>
std::string to_unicode(Network const& network)
{
	std::vector<std::string> lines(network.num_qubits(), "―");

	network.foreach_node([&](auto const& n) {
		auto const& g = n.gate;
		switch (g.kind()) {
		default:
			g.foreach_target([&](auto q) { lines[q] += "?"; });
			return;

		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			return;

		case gate_kinds_t::identity:
			g.foreach_target([&](auto q) { lines[q] += "I"; });
			break;

		case gate_kinds_t::hadamard:
			g.foreach_target([&](auto q) { lines[q] += "H"; });
			break;

		case gate_kinds_t::pauli_x:
			g.foreach_target([&](auto q) { lines[q] += "X"; });
			break;

		case gate_kinds_t::pauli_y:
			g.foreach_target([&](auto q) { lines[q] += "Y"; });
			break;

		case gate_kinds_t::pauli_z:
			g.foreach_target([&](auto q) { lines[q] += "Z"; });
			break;

		case gate_kinds_t::rotation_x:
			g.foreach_target([&](auto q) { lines[q] += "x"; });
			break;

		case gate_kinds_t::rotation_y:
			g.foreach_target([&](auto q) { lines[q] += "y"; });
			break;

		case gate_kinds_t::rotation_z:
			g.foreach_target([&](auto q) { lines[q] += "z"; });
			break;

		case gate_kinds_t::phase:
			g.foreach_target([&](auto q) { lines[q] += "S"; });
			break;

		case gate_kinds_t::phase_dagger:
			g.foreach_target([&](auto q) { lines[q] += "Ƨ"; });
			break;

		case gate_kinds_t::t:
			g.foreach_target([&](auto q) { lines[q] += "T"; });
			break;

		case gate_kinds_t::t3:
			g.foreach_target([&](auto q) { lines[q] += "3"; });
			break;

		case gate_kinds_t::t5:
			g.foreach_target([&](auto q) { lines[q] += "5"; });
			break;

		case gate_kinds_t::t_dagger:
			g.foreach_target([&](auto q) { lines[q] += "⊥"; });
			break;

		case gate_kinds_t::cx:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					lines[qc] += "●";
					lines[qt] += "⊕";
				});
			});
			break;

		case gate_kinds_t::cz:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					lines[qc] += "●";
					lines[qt] += "Z";
				});
			});
			break;

		case gate_kinds_t::mcx:
			g.foreach_control([&](auto q) { lines[q] += "●"; });
			g.foreach_target([&](auto q) { lines[q] += "⊕"; });
			break;

		case gate_kinds_t::mcy:
			g.foreach_control([&](auto q) { lines[q] += "●"; });
			g.foreach_target([&](auto q) { lines[q] += "Y"; });
			break;

		case gate_kinds_t::mcz:
			g.foreach_control([&](auto q) { lines[q] += "●"; });
			g.foreach_target([&](auto q) { lines[q] += "Z"; });
			break;
		}

		for (auto& line : lines) {
			if (line.size() % 2 == 0) {
				line += "―";
			} else {
				line += "――";
			}
		}
	});

	std::string result;
	for (auto const& line : lines) {
		result += line + "\n";
	}
	return result;
} // namespace tweedledum

} // namespace tweedledum
