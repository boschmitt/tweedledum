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
void write_qasm(Network const& circ, std::ostream& out)
{
	// header
	out << "OPENQASM 2.0;\n";
	out << "include \"qelib1.inc\";\n";
	out << fmt::format("qreg q[{}];\n", circ.num_qubits());
	out << fmt::format("creg c[{}];\n", circ.num_qubits());

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
				out << fmt::format("h q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::pauli_x: {
			g.foreach_target([&](auto q) {
				out << fmt::format("x q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::pauli_z: {
			g.foreach_target([&](auto q) {
				out << fmt::format("z q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::phase: {
			g.foreach_target([&](auto q) {
				out << fmt::format("s q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::phase_dagger: {
			g.foreach_target([&](auto q) {
				out << fmt::format("sdg q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::t: {
			g.foreach_target([&](auto q) {
				out << fmt::format("t q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::t_dagger: {
			g.foreach_target([&](auto q) {
				out << fmt::format("tdg q[{}];\n", q);
			});
		} break;

		case gate_kinds_t::rotation_z: {
			g.foreach_target([&](auto qt) {
				out << fmt::format("rz({}) q[{}];\n", g.angle(),
				                   qt);
			});
		} break;

		case gate_kinds_t::cx: {
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					out << fmt::format("cx q[{}],q[{}];\n",
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
					out << fmt::format("x q[{}];\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					out << fmt::format("cx q[{}],q[{}];\n",
					                   controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format("cx q[{}],q[{}];\n",
					                   targets[0],
					                   targets[i]);
				}
				out << fmt::format("ccx q[{}],q[{}],q[{}];\n",
				                   controls[0], controls[1],
				                   targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format("cx q[{}],q[{}];\n",
					                   targets[0],
					                   targets[i]);
				}
				break;
			}
		} break;
		}

		return true;
	});
}

template<typename Network>
void write_qasm(Network const& circ, const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ofstream::out);
	write_qasm(circ, out);
}

}; // namespace tweedledum
