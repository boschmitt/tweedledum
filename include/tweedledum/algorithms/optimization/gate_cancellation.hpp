/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include "../../representations/gate_kinds.hpp"

namespace tweedledum {

template<typename Network>
void single_qubit_gate_cancellation(Network& net)
{
	auto to_delete = 0u;
	net.foreach_gate([&net, &to_delete] (auto& node) {
		auto children = net.get_children(node, 0);
		for (auto child : children) {
			auto& child_node = net.get_node(child.index);
			if (net.mark(child_node)) {
				return;
			}
			if (node.gate.is(gate_adjoint(child_node.gate.kind()))) {
				net.mark(node, 1);
				net.mark(child_node, 1);
				to_delete += 2;
				return;
			}
		}
	});
	net.remove_marked_nodes();
	std::cout << "I deleted: " << to_delete << " gates.\n";
}

template<typename Network>
void two_qubit_gate_cancellation(Network& net)
{
	auto to_delete = 0u;
	net.foreach_gate([&net, &to_delete] (auto& node) {
		if (not node.gate.is(gate_kinds::cnot)) {
			return;
		}
		auto children = net.get_children(node, 0);
		for (auto child : children) {
			auto& child_node = net.get_node(child.index);
			if (net.mark(child_node) || not child_node.gate.is(gate_kinds::cnot)) {
				return;
			}
			if (node == child_node) {
				net.mark(node, 1);
				net.mark(child_node, 1);
				to_delete += 2;
				return;
			}
		}
	});
	net.remove_marked_nodes();
	std::cout << "I deleted: " << to_delete << " gates.\n";
}

} // namespace tweedledum

