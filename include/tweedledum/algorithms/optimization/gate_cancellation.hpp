/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/node.hpp"
#include "../../networks/wire.hpp"
#include "../transformation/remove_marked.hpp"

#include <cassert>
#include <vector>

namespace tweedledum {

/*! \brief Cancellation of consecutive adjoint gates.
 */
// TODO: still feels a bit hacky
template<typename Network>
Network gate_cancellation(Network const& network)
{
	using op_type = typename Network::op_type;
	using node_type = typename Network::node_type;

	uint32_t num_deletions = 0u;
	network.clear_values();
	network.foreach_op([&](op_type const& op, node_type const& node) {
		std::vector<node::id> children;
		network.foreach_child(node, [&](node_type const& child, wire::id const cwid) {
			node::id temp_nid = network.id(child);
			do {
				node_type const& ancestor = network.node(temp_nid);
				if (network.value(ancestor) == 1) {
					temp_nid = ancestor.children.at(ancestor.op.position(cwid));
					continue;
				}
				if (ancestor.op.is_meta() || ancestor.op.is_measurement()) {
					children.emplace_back(network.id(ancestor));
					return;
				}
				if (op.is_adjoint(ancestor.op) || op.is_dependent(ancestor.op)) {
					children.emplace_back(network.id(ancestor));
					return;
				}
				temp_nid = ancestor.children.at(ancestor.op.position(cwid));
			} while (1);
		});
		assert(children.size() == node.op.num_wires());
		bool do_remove = true;
		// Check if all children are equal
		for (uint32_t i = 1u; i < children.size(); ++i) {
			if (children.at(i - 1) != children.at(i)) {
				do_remove = false;
				break;
			}
		}
		if (do_remove) {
			node_type const& child = network.node(children.at(0u));
			if (child.op.is_meta() || child.op.is_measurement()) {
				return;
			}
			if (!op.is_adjoint(child.op)) {
				return;
			}
			network.value(node, 1u);
			network.value(child, 1u);
			num_deletions += 2;
		}
	});
	if (num_deletions == 0) {
		return network;
	}
	return remove_marked(network);
}

} // namespace tweedledum