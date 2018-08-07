/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

namespace tweedledum {

template<class NetworkDest, class NetworkSrc, class RewriteFn>
void rewrite_network(NetworkDest& dest, NetworkSrc const& src, RewriteFn&& fn, uint32_t ancillae = 0)
{
	for (auto i = 0u; i < src.num_qubits() + ancillae; ++i) {
		dest.allocate_qubit();
	}

	src.foreach_node([&](auto const& n) {
		if (!fn(dest, n.gate)) {
			if constexpr (std::is_same_v<typename NetworkSrc::gate_type,
			                             typename NetworkDest::gate_type>) {
				dest.add_gate(n.gate);
			}
		}
	});
}

} // namespace tweedledum
