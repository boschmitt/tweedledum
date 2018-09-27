/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_kinds.hpp"
#include "../../views/immutable_view.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace tweedledum {


/*! \brief TODO
 *
 * **Required network functions:**
 * - `add_gate`
 * - `foreach_qubit`
 * - `foreach_gate`
 * - `mark`
 */
template<typename NetworkSrc, typename NetworkDest>
void single_qubit_gate_cancellation(NetworkSrc& src, NetworkDest& dest)
{
	immutable_view immutable_net(src);
	(void) dest;
	std::cout << "Teste " << immutable_net.size() << "\n"; 
}

/*! \brief TODO
 *
 * **Required network functions:**
 * - `add_gate`
 * - `foreach_qubit`
 * - `foreach_gate`
 * - `mark`
 */
template<typename Network>
Network single_qubit_gate_cancellation(Network &src)
{
	Network dest;
	src.foreach_qubit([&](auto id, auto& qubit_label) {
		(void) id;
		dest.add_qubit(qubit_label);
	});
	single_qubit_gate_cancellation(src, dest);
	return dest;
}

// template<typename Network>
// void single_qubit_gate_cancellation(Network& net, bool remove_gates = true)
// {
// 	auto to_delete = 0u;
// 	net.foreach_node([&net, &to_delete](auto& node) {
// 		if (node.gate.is_controlled()) {
// 			return;
// 		}
// 		auto children = net.get_children(node, node.gate.target());
// 		std::cout << "Childs: " << children.size() << "\n";
// 		for (auto child : children) {
// 			auto& child_node = net.get_node(child);
// 			if (net.mark(child_node)) {
// 				continue;
// 			}
// 			if (node.gate.is(gate_adjoint(child_node.gate.kind()))) {
// 				net.mark(node, 1);
// 				net.mark(child_node, 1);
// 				to_delete += 2;
// 				return;
// 			}
// 		}
// 	});
// 	if (remove_gates) {
// 		net.remove_marked_nodes();
// 	}
// 	std::cout << "I deleted: " << to_delete << " gates.\n";
// }

} // namespace tweedledum
