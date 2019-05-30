/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
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
	network.clear_values();
	network.foreach_gate([&](auto const& node) {
		std::vector<link_type> children;
		bool do_remove = false;
		network.foreach_child(node, [&](auto child_index) {
			if (!children.empty() && children.back() != child_index) {
				do_remove = false;
				return false;
			}
			children.push_back(child_index);
			auto& child = network.node(child_index);
			if (network.value(child)) {
				return true;
			}
			if (node.gate.is_adjoint(child.gate)) {
				do_remove = true;
			}
			return true;
		});
		if (do_remove) {
			network.set_value(node, 1);
			network.set_value(network.node(children.back()), 1);
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