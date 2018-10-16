/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cassert>

namespace tweedledum {

template<class NetworkDest, class NetworkSrc, class RewriteFn>
void rewrite_network(NetworkDest& dest, NetworkSrc const& src, RewriteFn&& fn, uint32_t ancillae = 0)
{
	assert(dest.size() == 0);
	src.foreach_qubit([&](auto id, auto& qubit_label) {
		(void) id;
		dest.add_qubit(qubit_label);
	});
	for (auto i = 0u; i < ancillae; ++i) {
		dest.add_qubit();
	}

	src.foreach_gate([&](auto const& node) {
		if (!fn(dest, node.gate)) {
			if constexpr (std::is_same_v<typename NetworkSrc::gate_type,
			                             typename NetworkDest::gate_type>) {
				dest.add_gate(node.gate);
			}
		}
	});
}

template<class NetworkDest, class NetworkSrc, class RewriteFn>
NetworkDest rewrite_network(NetworkSrc const& src, RewriteFn&& fn, uint32_t ancillae = 0)
{
	NetworkDest dest;
	src.foreach_qubit([&](auto id, auto& qubit_label) {
		(void) id;
		dest.add_qubit(qubit_label);
	});
	for (auto i = 0u; i < ancillae; ++i) {
		dest.add_qubit();
	}

	src.foreach_node([&](auto const& n) {
		if (!fn(dest, n.gate)) {
			dest.add_gate(n.gate);
		}
	});
	return dest;
}

} // namespace tweedledum
