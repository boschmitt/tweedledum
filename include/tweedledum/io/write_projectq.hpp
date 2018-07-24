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

auto make_qubit_list(std::string& s)
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

		char u;
		switch (g.kind()) {
		default:
			assert(false);
			break;
		case gate_kinds_t::mcx:
			u = 'X';
			break;
		case gate_kinds_t::mcy:
			u = 'Y';
			break;
		case gate_kinds_t::mcz:
			u = 'Z';
			break;
		}

		out << fmt::format("C(All({}), {}) | ([{}], [{}])\n", u,
		                   g.num_controls(), controls, targets);
	});
}

template<typename Network>
void write_projectq(Network const& circ, const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ofstream::out);
	write_projectq(circ, out);
}

}; // namespace tweedledum
