/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../networks/netlist.hpp"

#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <kitty/detail/mscfix.hpp>
#include <string>

namespace tweedledum {

inline auto make_qubit_list(std::string& s)
{
	return [&](auto c) {
		if (!s.empty()) {
			s += ", ";
		}
		s += fmt::format("qs[{}]", c);
	};
}

template<typename Network>
void write_projectq(Network const& circ, std::ostream& out)
{
	circ.foreach_node([&](auto const& n) {
		auto const& g = n.gate;

		std::string controls, targets;

		g.foreach_control(make_qubit_list(controls));
		g.foreach_target(make_qubit_list(targets));

		switch (g.kind()) {
		default:
			std::cout << "[w] unknown gate kind "
			          << static_cast<uint32_t>(g.kind()) << "\n";
			assert(false);
			break;
		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			break;
		case gate_kinds_t::hadamard:
			out << fmt::format("H | {}\n", targets);
			break;
		case gate_kinds_t::pauli_x:
			out << fmt::format("X | {}\n", targets);
			break;
		case gate_kinds_t::pauli_y:
			out << fmt::format("Y | {}\n", targets);
			break;
		case gate_kinds_t::pauli_z:
			out << fmt::format("Z | {}\n", targets);
			break;
		case gate_kinds_t::phase:
			out << fmt::format("S | {}\n", targets);
			break;
		case gate_kinds_t::phase_dagger:
			out << fmt::format("Sdag | {}\n", targets);
			break;
		case gate_kinds_t::t:
			out << fmt::format("T | {}\n", targets);
			break;
		case gate_kinds_t::t_dagger:
			out << fmt::format("Tdag | {}\n", targets);
			break;
		case gate_kinds_t::rotation_x:
			out << fmt::format("Rx({}) | {}\n", g.angle(), targets);
			break;
		case gate_kinds_t::rotation_z:
			out << fmt::format("Rz({}) | {}\n", g.angle(), targets);
			break;
		case gate_kinds_t::cx:
			out << fmt::format("CNOT | ({}, {})\n", controls,
			                   targets);
			break;
		case gate_kinds_t::cz:
			out << fmt::format("CZ | ({}, {})\n", controls, targets);
			break;
		case gate_kinds_t::mcx:
			out << fmt::format("C(All(X), {}) | ([{}], [{}])\n",
			                   g.num_controls(), controls, targets);
			break;
		case gate_kinds_t::mcy:
			out << fmt::format("C(All(Y), {}) | ([{}], [{}])\n",
			                   g.num_controls(), controls, targets);
			break;
		case gate_kinds_t::mcz:
			out << fmt::format("C(All(Z), {}) | ([{}], [{}])\n",
			                   g.num_controls(), controls, targets);
			break;
		}
	});
}

template<typename Network>
void write_projectq(Network const& circ, const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ofstream::out);
	write_projectq(circ, out);
}

}; // namespace tweedledum
