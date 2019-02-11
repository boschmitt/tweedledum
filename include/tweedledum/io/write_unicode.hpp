/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_set.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace tweedledum {

/*! \brief Creates a Unicode string that represent the network
 *
 * **Required gate functions:**
 * - `op`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \return A string representing the network using Unicode characters
 */
template<class Network>
std::string to_unicode_str(Network const& network)
{
	std::vector<std::string> lines(network.num_qubits(), "―");

	network.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			gate.foreach_target([&](auto qid) { lines[qid] += "?"; });
			return;

		case gate_set::identity:
			gate.foreach_target([&](auto qid) { lines[qid] += "I"; });
			break;

		case gate_set::hadamard:
			gate.foreach_target([&](auto qid) { lines[qid] += "H"; });
			break;

		case gate_set::pauli_x:
			gate.foreach_target([&](auto qid) { lines[qid] += "X"; });
			break;

		case gate_set::pauli_y:
			gate.foreach_target([&](auto qid) { lines[qid] += "Y"; });
			break;

		case gate_set::pauli_z:
			gate.foreach_target([&](auto qid) { lines[qid] += "Z"; });
			break;

		case gate_set::rotation_x:
			gate.foreach_target([&](auto qid) { lines[qid] += "x"; });
			break;

		case gate_set::rotation_y:
			gate.foreach_target([&](auto qid) { lines[qid] += "y"; });
			break;

		case gate_set::rotation_z:
			gate.foreach_target([&](auto qid) { lines[qid] += "z"; });
			break;

		case gate_set::phase:
			gate.foreach_target([&](auto qid) { lines[qid] += "S"; });
			break;

		case gate_set::phase_dagger:
			gate.foreach_target([&](auto qid) { lines[qid] += "Ƨ"; });
			break;

		case gate_set::t:
			gate.foreach_target([&](auto qid) { lines[qid] += "T"; });
			break;

		case gate_set::t_dagger:
			gate.foreach_target([&](auto qid) { lines[qid] += "⊥"; });
			break;

		case gate_set::cx:
		case gate_set::mcx:
			gate.foreach_control([&](auto qid_control) { 
				if (qid_control.is_complemented()) {
					lines[qid_control] += "○";
				} else {
					lines[qid_control] += "●";
				}
			});
			gate.foreach_target([&](auto qid_target) { lines[qid_target] += "⊕"; });
			break;

		case gate_set::cz:
		case gate_set::mcz:
			gate.foreach_control([&](auto qid_control) { 
				if (qid_control.is_complemented()) {
					lines[qid_control] += "○";
				} else {
					lines[qid_control] += "●";
				}
			});
			gate.foreach_target([&](auto qid_target) { lines[qid_target] += "Z"; });
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
}

/*! \brief Writes a network in Unicode format format into a output stream
 *
 * **Required gate functions:**
 * - `op`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param os Output stream (default: std::cout)
 */
template<typename Network>
void write_unicode(Network const& network, std::ostream& os = std::cout)
{
	auto unicode_str = to_unicode_str(network);
	os << unicode_str;
}

/*! \brief Writes a network in Unicode format format into a file
 *
 * **Required gate functions:**
 * - `op`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param filename Filename
 */
template<typename Network>
void write_unicode(Network const& network, std::string const& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_unicode(network, os);
}

} // namespace tweedledum
