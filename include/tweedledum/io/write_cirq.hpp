/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "../networks/gates/gate_kinds.hpp"
#include "../networks/netlist.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

namespace tweedledum {

template<typename Network>
void write_cirq(Network const& circ, std::ostream& out)
{
	circ.foreach_node([&](auto const& n) {
		auto const& g = n.gate;
		switch (g.kind()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;

		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			return true;

		case gate_kinds_t::hadamard: {
			g.foreach_target([&](auto q) {
				out << fmt::format(
				    "qc.append(cirq.H(qs[{}]))\n", q);
			});
		} break;

		case gate_kinds_t::pauli_x: {
			g.foreach_target([&](auto q) {
				out << fmt::format(
				    "qc.append(cirq.X(qs[{}]))\n", q);
			});
		} break;

		case gate_kinds_t::t: {
			g.foreach_target([&](auto q) {
				out << fmt::format(
				    "qc.append(cirq.T(qs[{}]))\n", q);
			});
		} break;

		case gate_kinds_t::t_dagger: {
			g.foreach_target([&](auto q) {
				out << fmt::format(
				    "qc.append(cirq.RotZGate(rads=5."
				    "497787143782138)(qs[{}]))\n",
				    q);
			});
		} break;

		case gate_kinds_t::rotation_z: {
			g.foreach_target([&](auto q) {
				out << fmt::format("qc.append(cirq.RotZGate("
				                   "rads={})(qs[{}]))\n",
				                   g.angle(), q);
			});
		} break;

		case gate_kinds_t::cx: {
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					out << fmt::format(
					    "qc.append(cirq.CNOT(qs[{}], "
					    "qs[{}]))\n",
					    qc, qt);
				});
			});
		} break;

		case gate_kinds_t::cz: {
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					out << fmt::format("qc.append(cirq.CZ("
					                   "qs[{}], qs[{}]))\n",
					                   qc, qt);
				});
			});
		} break;

		case gate_kinds_t::mcx: {
			std::vector<uint32_t> controls, targets;
			g.foreach_control([&](auto q) { controls.push_back(q); });
			g.foreach_target([&](auto q) { targets.push_back(q); });
			switch (controls.size()) {
			default:
				std::cerr << "[w] unsupported control size\n";
				return true;
			case 0u:
				for (auto q : targets) {
					out << fmt::format(
					    "qc.append(cirq.X(qs[{}]))\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					out << fmt::format(
					    "qc.append(cirq.CNOT(qs[{}], "
					    "qs[{}]))\n",
					    controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format(
					    "qc.append(cirq.CNOT(qs[{}], "
					    "qs[{}]))\n",
					    targets[0], targets[i]);
				}
				out << fmt::format("qc.append(cirq.CCX(qs[{}], "
				                   "qs[{}], qs[{}]))\n",
				                   controls[0], controls[1],
				                   targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format(
					    "qc.append(cirq.CNOT(qs[{}], "
					    "qs[{}]))\n",
					    targets[0], targets[i]);
				}
				break;
			}
		} break;
		}

		return true;
	});
}

template<typename Network>
void write_cirq(Network const& circ, const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ofstream::out);
	write_cirq(circ, out);
}
}; // namespace tweedledum
