/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../generic/remove_marked.hpp"
#include "../../gates/gate_lib.hpp"

#include <vector>
#include <set>
#include <cassert>

#include <fmt/format.h>
#include <fmt/ostream.h>

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
		std::set<uint32_t> qubits;
		network.foreach_child(node, [&](link_type child_index, io_id io) {
			children.emplace_back(child_index);
			qubits.insert(io.index());
		});
		size_t i = 0;
		size_t i_end = children.size();
		link_type adj_node;
		bool do_remove = false;
		while(i < children.size()) {
			auto prev_node = network.get_node(children.at(i));
			if (prev_node.gate.is(gate_lib::input)) {
				break;
			}
			if (network.value(prev_node) == 0) {
				if (node.gate.is_adjoint(prev_node.gate)) {
					adj_node = children.at(i);
					do_remove = true;
					++i;
					continue;
				}
				if (node.gate.is_dependent(prev_node.gate)) {
					do_remove = false;
					break;
				}
			}
			network.foreach_child(prev_node, [&](link_type child_index, io_id io) {
				if (qubits.find(io.index()) == qubits.end()) {
					return;
				}
				children.emplace_back(child_index);
			});
			++i;
			if (i == i_end) {
				if (do_remove) {
					break;
				}
				i_end = children.size();
			}
		}
		if (do_remove) {
			network.set_value(node, 1);
			network.set_value(network.get_node(adj_node), 1);
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