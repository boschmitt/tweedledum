/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"
#include "../networks/netlist.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

namespace tweedledum {

/*! \brief Writes network in CirQ format into output stream
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
void write_cirq(Network const& network, std::ostream& os)
{
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
			g.foreach_target(
			    [&](auto q) { os << fmt::format("qc.append(cirq.H(qs[{}]))\n", q); });
			break;

		case gate_kinds_t::pauli_x:
			g.foreach_target(
			    [&](auto q) { os << fmt::format("qc.append(cirq.X(qs[{}]))\n", q); });
			break;

		case gate_kinds_t::t:
			g.foreach_target(
			    [&](auto q) { os << fmt::format("qc.append(cirq.T(qs[{}]))\n", q); });
			break;

		case gate_kinds_t::t_dagger:
			g.foreach_target([&](auto q) {
				os << fmt::format("qc.append(cirq.RotZGate(rads=5."
				                  "497787143782138)(qs[{}]))\n",
				                  q);
			});
			break;

		case gate_kinds_t::rotation_z:
			g.foreach_target([&](auto q) {
				os << fmt::format("qc.append(cirq.RotZGate("
				                  "rads={})(qs[{}]))\n",
				                  g.rotation_angle(), q);
			});
			break;

		case gate_kinds_t::cx:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					os << fmt::format("qc.append(cirq.CNOT(qs[{}], "
					                  "qs[{}]))\n",
					                  qc, qt);
				});
			});
			break;

		case gate_kinds_t::cz:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					os << fmt::format("qc.append(cirq.CZ("
					                  "qs[{}], qs[{}]))\n",
					                  qc, qt);
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
					os << fmt::format("qc.append(cirq.X(qs[{}]))\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					os << fmt::format("qc.append(cirq.CNOT(qs[{}], "
					                  "qs[{}]))\n",
					                  controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("qc.append(cirq.CNOT(qs[{}], "
					                  "qs[{}]))\n",
					                  targets[0], targets[i]);
				}
				os << fmt::format("qc.append(cirq.CCX(qs[{}], "
				                  "qs[{}], qs[{}]))\n",
				                  controls[0], controls[1], targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("qc.append(cirq.CNOT(qs[{}], "
					                  "qs[{}]))\n",
					                  targets[0], targets[i]);
				}
				break;
			}
			break;
		}
		return true;
	});
}

/*! \brief Writes network in CirQ format into a file
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
void write_cirq(Network const& network, const std::string& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_cirq(network, os);
}
} // namespace tweedledum
