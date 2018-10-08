/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

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
template<typename Network>
void write_qasm(Network const& network, std::ostream& os)
{
	// header
	os << "OPENQASM 2.0;\n";
	os << "include \"qelib1.inc\";\n";
	os << fmt::format("qreg q[{}];\n", network.num_qubits());
	os << fmt::format("creg c[{}];\n", network.num_qubits());

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
			g.foreach_target([&](auto q) { os << fmt::format("h q[{}];\n", q); });
			break;

		case gate_kinds_t::pauli_x:
			g.foreach_target([&](auto q) { os << fmt::format("x q[{}];\n", q); });
			break;

		case gate_kinds_t::pauli_z:
			g.foreach_target([&](auto q) { os << fmt::format("z q[{}];\n", q); });
			break;

		case gate_kinds_t::phase:
			g.foreach_target([&](auto q) { os << fmt::format("s q[{}];\n", q); });
			break;

		case gate_kinds_t::phase_dagger:
			g.foreach_target([&](auto q) { os << fmt::format("sdg q[{}];\n", q); });
			break;

		case gate_kinds_t::t:
			g.foreach_target([&](auto q) { os << fmt::format("t q[{}];\n", q); });
			break;

		case gate_kinds_t::t_dagger:
			g.foreach_target([&](auto q) { os << fmt::format("tdg q[{}];\n", q); });
			break;

		case gate_kinds_t::rotation_z:
			g.foreach_target([&](auto qt) {
				os << fmt::format("rz({}) q[{}];\n", g.rotation_angle(), qt);
			});
			break;

		case gate_kinds_t::cx:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					os << fmt::format("cx q[{}], q[{}];\n", qc, qt);
				});
			});
			break;

		case gate_kinds_t::mcx:
			std::vector<uint32_t> controls, targets;
			g.foreach_control([&](auto q) { controls.push_back(q); });
			g.foreach_target([&](auto q) { targets.push_back(q); });
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
 * - `kind`
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
template<typename Network>
void write_qasm(Network const& network, const std::string& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_qasm(network, os);
}

} // namespace tweedledum
