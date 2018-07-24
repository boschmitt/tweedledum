/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../networks/netlist.hpp"
#include "../networks/gates/gate_kinds.hpp"

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
		if (g.kind() != gate_kinds_t::mcx) {
			std::cerr << "[w] unsupported gate type\n";
			return true;
		}
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
				out << fmt::format("CNOT {} {}\n", controls[0],
				                   q);
			}
			break;
		case 2u:
			for (auto i = 1u; i < targets.size(); ++i) {
				out << fmt::format("CNOT {} {}\n", targets[0],
				                   targets[i]);
			}
			out << fmt::format("CCNOT {} {} {}\n", controls[0],
			                   controls[1], targets[0]);
			for (auto i = 1u; i < targets.size(); ++i) {
				out << fmt::format("CNOT {} {}\n", targets[0],
				                   targets[i]);
			}
			break;
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
