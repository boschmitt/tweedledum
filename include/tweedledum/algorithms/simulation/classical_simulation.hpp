/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <iostream>
#include <tweedledum/gates/gate_kinds.hpp>

namespace tweedledum {

template<typename Network>
uint64_t simulate_pattern_classical(Network& net, uint64_t pattern)
{
	net.foreach_node([&](auto const& n) {
		auto const& g = n.gate;
		switch (g.kind()) {
		default:
			std::cerr << "[w] no classical gate, abort simulation, and return 0\n";
			pattern = 0;
			return false;
		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			return true;
		case gate_kinds_t::pauli_x:
			g.foreach_target([&](auto q) { pattern ^= 1 << q; });
			break;
		case gate_kinds_t::cx:
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					if ((pattern >> qc) & 1) {
						pattern ^= 1 << qt;
					}
				});
			});
			break;
		case gate_kinds_t::mcx: {
			uint64_t cmask{0}, tmask{0};
			g.foreach_control([&](auto q) { cmask |= 1 << q; });
			g.foreach_target([&](auto q) { tmask |= 1 << q; });
			if ((pattern & cmask) == cmask) {
				pattern ^= tmask;
			}
		} break;
		}
		return true;
	});

	return pattern;
}

} // namespace tweedledum
