/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../generic/remove_marked.hpp"
#include "../../gates/gate_lib.hpp"
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
	network.clear_values();

	auto start = std::chrono::steady_clock::now();
	network.foreach_gate([&](auto const& node) {
		std::vector<link_type> children;
		std::set<uint32_t> qubits;
		network.foreach_child(node, [&](link_type child_index, io_id io) {
			do {
				auto prev_node = network.get_node(child_index);
				if (network.value(prev_node) == 1) {
					child_index = prev_node.children[prev_node.gate.qubit_slot(io)];
					continue;
				}
				if (node.gate.is_adjoint(prev_node.gate) || node.gate.is_dependent(prev_node.gate)) {
					children.emplace_back(child_index);
					return;
				}
				child_index = prev_node.children[prev_node.gate.qubit_slot(io)];
			} while (1);
		});
		assert(children.size() == node.gate.num_io());
		bool do_remove = true;
		for (uint32_t i = 1; i < children.size(); ++i) {
			if (children.at(i - 1) != children.at(i)) {
				do_remove = false;
				break;
			}
		}
		if (do_remove) {
			auto& adj_node = network.get_node(children[0]);
			if (!node.gate.is_adjoint(adj_node.gate)) {
				return;
			}
			network.set_value(node, 1);
			network.set_value(adj_node, 1);
			num_deletions += 2;
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