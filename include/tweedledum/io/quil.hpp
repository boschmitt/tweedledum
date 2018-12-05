/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_set.hpp"

#include <fmt/format.h>
#include <fstream>
#include <iostream>

namespace tweedledum {

/*! \brief Writes network in quil format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 *
 * **Required network functions:**
 * - `foreach_cnode`
 * - `foreach_cqubit`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param os Output stream
 */
template<typename Network>
void write_quil(Network const& network, std::ostream& os)
{
	network.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;

		case gate_set::hadamard:
			gate.foreach_target([&](auto q) { os << fmt::format("H {}\n", q); });
			break;

		case gate_set::pauli_x:
			gate.foreach_target([&](auto q) { os << fmt::format("X {}\n", q); });
			break;

		case gate_set::t:
			gate.foreach_target([&](auto q) { os << fmt::format("T {}\n", q); });
			break;

		case gate_set::t_dagger:
			gate.foreach_target([&](auto q) { os << fmt::format("RZ(-pi/4) {}\n", q); });
			break;

		case gate_set::rotation_z:
			gate.foreach_target([&](auto qt) {
				os << fmt::format("RZ({}) {}\n", gate.rotation_angle().numeric_value(), qt);
			});
			break;

		case gate_set::cx:
			gate.foreach_control([&](auto qc) {
				gate.foreach_target([&](auto qt) {
					os << fmt::format("CNOT {} {}\n", qc, qt); 
				});
			});
			break;

		case gate_set::mcx:
			std::vector<uint32_t> controls, targets;
			gate.foreach_control([&](auto q) { controls.push_back(q); });
			gate.foreach_target([&](auto q) { targets.push_back(q); });
			switch (controls.size()) {
			default:
				std::cerr << "[w] unsupported control size\n";
				return true;
			case 0u:
				for (auto q : targets) {
					os << fmt::format("X {}\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					os << fmt::format("CNOT {} {}\n", controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("CNOT {} {}\n", targets[0], targets[i]);
				}
				os << fmt::format("CCNOT {} {} {}\n", controls[0], controls[1],
				                   targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("CNOT {} {}\n", targets[0], targets[i]);
				}
				break;
			}
			break;
		}
		return true;
	});
}

/*! \brief Writes network in quil format into a file
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 *
 * **Required network functions:**
 * - `foreach_cnode`
 * - `foreach_cqubit`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param filename Filename
 */
template<typename Network>
void write_quil(Network const& network, std::string const& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_quil(network, os);
}

} // namespace tweedledum
