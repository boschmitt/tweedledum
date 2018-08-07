/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../networks/gates/gate_kinds.hpp"
#include "../networks/netlist.hpp"

#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

namespace tweedledum {

template<typename Network>
void write_quil(Network const& circ, std::ostream& out)
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
			g.foreach_target(
			    [&](auto q) { out << fmt::format("H {}\n", q); });
		} break;

		case gate_kinds_t::pauli_x: {
			g.foreach_target(
			    [&](auto q) { out << fmt::format("X {}\n", q); });
		} break;

		case gate_kinds_t::t: {
			g.foreach_target(
			    [&](auto q) { out << fmt::format("T {}\n", q); });
		} break;

		case gate_kinds_t::t_dagger: {
			g.foreach_target([&](auto q) {
				out << fmt::format("RZ(-pi/4) {}\n", q);
			});
		} break;

		case gate_kinds_t::cx: {
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					out << fmt::format("CNOT {} {}\n", qc,
					                   qt);
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
					out << fmt::format("X {}\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					out << fmt::format("CNOT {} {}\n",
					                   controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format("CNOT {} {}\n",
					                   targets[0],
					                   targets[i]);
				}
				out << fmt::format("CCNOT {} {} {}\n",
				                   controls[0], controls[1],
				                   targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format("CNOT {} {}\n",
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
void write_quil(Network const& circ, const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ofstream::out);
	write_quil(circ, out);
}

}; // namespace tweedledum
