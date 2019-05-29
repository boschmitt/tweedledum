/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../generic/remove_marked.hpp"
#include <cassert>

namespace tweedledum {

/*! \brief Cancellation of consecutive adjoint gates.
 */
// TODO: still feels a bit hacky
template<typename Network>
Network gate_cancellation(Network const& network)
{
	using link_type = typename Network::link_type;
	uint32_t num_deletions = 0u;
	network.clear_visited();
	network.foreach_gate([&](auto const& vertex) {
		std::vector<link_type> children;
		bool do_remove = false;
		network.foreach_child(vertex, [&](auto child_index) {
			if (!children.empty() && children.back() != child_index) {
				do_remove = false;
				return false;
			}
			children.push_back(child_index);
			auto& child = network.vertex(child_index);
			if (network.visited(child)) {
				return true;
			}
			if (vertex.gate.is_adjoint(child.gate)) {
				do_remove = true;
			}
			return true;
		});
		if (do_remove) {
			network.set_visited(vertex, 1);
			network.set_visited(network.vertex(children.back()), 1);
			num_deletions += 2;
			return;
		}
	});
	if (num_deletions == 0) {
		return network;
	}
	return remove_marked(network);
}

} // namespace tweedledum