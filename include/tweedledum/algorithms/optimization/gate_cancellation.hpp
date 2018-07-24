/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace tweedledum {

template<typename Network>
void single_qubit_gate_cancellation(Network& net, bool remove_gates = true)
{
	auto to_delete = 0u;
	net.foreach_gate([&net, &to_delete](auto& node) {
		if (node.gate.is_controlled()) {
			return;
		}
		auto children = net.get_children(node, node.gate.target());
		for (auto child : children) {
			auto& child_node = net.get_node(child);
			if (net.mark(child_node)) {
				continue;
			}
			if (node.gate.is(gate_adjoint(child_node.gate.kind()))) {
				net.mark(node, 1);
				net.mark(child_node, 1);
				to_delete += 2;
				return;
			}
		}
	});
	if (remove_gates) {
		net.remove_marked_nodes();
	}
	std::cout << "I deleted: " << to_delete << " gates.\n";
}

template<typename Network>
void controlled_gate_cancellation(Network& net, bool remove_gates = true)
{
	auto to_delete = 0u;
	net.foreach_gate([&net, &to_delete](auto& node) {
		if (node.gate.is_controlled() == false) {
			return;
		}
		auto children = net.get_children(node, node.gate.target());
		for (auto child : children) {
			auto& child_node = net.get_node(child);
			if (net.mark(child_node) || node.gate.is_controlled() == false) {
				continue;
			}
			if (node == child_node) {
				net.mark(node, 1);
				net.mark(child_node, 1);
				to_delete += 2;
				return;
			}
		}
	});
	if (remove_gates) {
		net.remove_marked_nodes();
	}
	std::cout << "I deleted: " << to_delete << " gates.\n";
}

} // namespace tweedledum
