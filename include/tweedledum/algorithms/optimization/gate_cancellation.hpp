/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../generic/remove_marked.hpp"
#include "../../gates/gate_lib.hpp"
#include "../../utils/dynamic_bitset.hpp"
#include "../../utils/stopwatch.hpp"

#include <vector>
#include <set>
#include <cassert>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>

namespace tweedledum {

/*! \brief Cancellation of consecutive adjoint gates.
 */
// TODO: still feels a bit hacky
template<typename Network>
Network gate_cancellation(Network const& network, nlohmann::json* stats = nullptr)
{
	using link_type = typename Network::link_type;
	uint32_t num_deletions = 0u;
	dynamic_bitset<uint32_t> seen_children(network.size());
	network.clear_values();

	auto start = std::chrono::steady_clock::now();
	network.foreach_gate([&](auto const& node) {
		std::vector<link_type> children;
		std::set<uint32_t> qubits;
		seen_children.reset();
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
				if (seen_children[child_index] == 0) {
					children.emplace_back(child_index);
					seen_children[child_index] = 1;
				}
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
	auto elapsed_time = std::chrono::steady_clock::now() - start;
	if (stats) {
		(*stats)["passes"] += { {"pass", "gate_cancellation"}, 
		                        {"time", to_seconds(elapsed_time) },
					{"deletions", num_deletions } };
	}
	if (num_deletions == 0) {
		return network;
	}
	return remove_marked(network);
}

} // namespace tweedledum