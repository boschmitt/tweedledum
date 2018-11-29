/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_set.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

namespace tweedledum {

/*! \brief Writes network in OPENQASM 2.0 format into output stream
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
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param os Output stream
 */
template<typename Network>
void write_qasm(Network const& network, std::ostream& os)
{
	// header
	os << "OPENQASM 2.0;\n";
	os << "include \"qelib1.inc\";\n";
	os << fmt::format("qreg q[{}];\n", network.num_qubits());
	os << fmt::format("creg c[{}];\n", network.num_qubits());

	network.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.op()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;

		case gate_set::hadamard:
			gate.foreach_target([&](auto q) { os << fmt::format("h q[{}];\n", q); });
			break;

		case gate_set::pauli_x:
			gate.foreach_target([&](auto q) { os << fmt::format("x q[{}];\n", q); });
			break;

		case gate_set::pauli_z:
			gate.foreach_target([&](auto q) { os << fmt::format("z q[{}];\n", q); });
			break;

		case gate_set::phase:
			gate.foreach_target([&](auto q) { os << fmt::format("s q[{}];\n", q); });
			break;

		case gate_set::phase_dagger:
			gate.foreach_target([&](auto q) { os << fmt::format("sdg q[{}];\n", q); });
			break;

		case gate_set::t:
			gate.foreach_target([&](auto q) { os << fmt::format("t q[{}];\n", q); });
			break;

		case gate_set::t_dagger:
			gate.foreach_target([&](auto q) { os << fmt::format("tdg q[{}];\n", q); });
			break;

		case gate_set::rotation_z:
			gate.foreach_target([&](auto qt) {
				os << fmt::format("rz({}) q[{}];\n", gate.rotation_angle().numeric_value(), qt);
			});
			break;

		case gate_set::cx:
			gate.foreach_control([&](auto qc) {
				gate.foreach_target([&](auto qt) {
					os << fmt::format("cx q[{}], q[{}];\n", qc, qt);
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
					os << fmt::format("x q[{}];\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					os << fmt::format("cx q[{}],q[{}];\n", controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("cx q[{}], q[{}];\n", targets[0],
					                   targets[i]);
				}
				os << fmt::format("ccx q[{}], q[{}], q[{}];\n", controls[0],
				                   controls[1], targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("cx q[{}], q[{}];\n", targets[0],
					                   targets[i]);
				}
				break;
			}
			break;
		}
		return true;
	});
}

/*! \brief Writes network in OPENQASM 2.0 format into a file
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 * 
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_cnode`
 *
 * \param network A quantum network
 * \param filename Filename
 */
template<typename Network>
void write_qasm(Network const& network, const std::string& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_qasm(network, os);
}

} // namespace tweedledum
