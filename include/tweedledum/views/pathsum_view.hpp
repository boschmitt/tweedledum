/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../traits.hpp"
#include "../utils/hash.hpp"
#include "../utils/node_map.hpp"
#include "immutable_view.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

namespace tweedledum {

/*! \brief This view computes the path sums of each node of the network.
 *
 * It implements the network interface methods `get_pathsum`.  The pathsums are computed at
 * construction. The network must be on basis {CX, Rz, H}
 *
 * **Required gate functions:**
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `foreach_cinput`
 * - `foreach_coutput`
 */
template<typename Network>
class pathsum_view : public immutable_view<Network> {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using node_ptr_type = typename Network::node_ptr_type;
	using storage_type = typename Network::storage_type;

	using esop_type = std::set<uint32_t>;

	explicit pathsum_view(Network& network)
	    : immutable_view<Network>(network)
	    , pathsum_to_node_()
	    , node_to_pathsum_(network)
	    , num_path_vars_(network.num_qubits() + 1)
	    , qubit_state_(this->num_qubits())
	    , phy_virtual_map_(network.num_qubits(), 0)
	{
		std::iota(phy_virtual_map_.begin(), phy_virtual_map_.end(), 0);
		compute_pathsums();
	}

	// Pathsums can be employed to verify mappings.  Assuming that the mapping do not screw-up
	// adding single qubit gates, we can safely ignore them and verify if the set of output path sums
	// of the original circuit mataches the set of output path sums of the mapped circuit.
	//
	// The user need to pass the _initial_ physical->virtual mapping so that the path literals
	// can be placed correctly.
	explicit pathsum_view(Network& network, std::vector<uint32_t> phy_virtual_map)
	    : immutable_view<Network>(network)
	    , pathsum_to_node_()
	    , node_to_pathsum_(network)
	    , num_path_vars_(network.num_qubits() + 1)
	    , qubit_state_(this->num_qubits())
	    , phy_virtual_map_(phy_virtual_map.size(), 0)
	{
		for (uint32_t i = 0; i < phy_virtual_map.size(); ++i) {
			phy_virtual_map_[phy_virtual_map[i]] = i;
		}
		compute_pathsums();
	}

	/*! \brief Returns the path equations of a node. */
	auto& get_pathsum(node_type const& node) const
	{
		return node_to_pathsum_[node]->first;
	}

private:
	void map_pathsum_to_node(qubit_id qid, node_type const& node, uint32_t node_index)
	{
		const std::vector<uint32_t> node_list = {node_index};
		auto map_element = pathsum_to_node_.emplace(
		    std::make_pair(qubit_state_[qid], node_list));
		if (map_element.second == false) {
			map_element.first->second.push_back(node_index);
		}
		node_to_pathsum_[node] = map_element.first;
	}

	void compute_pathsums()
	{
		// Initialize qubit_state_ with initial path literals
		this->foreach_cinput([&](auto& node, auto node_index) {
			const auto path_literal = ((phy_virtual_map_[node_index] + 1) << 1);
			qubit_state_[node_index].emplace(path_literal);
			map_pathsum_to_node(node_index, node, node_index);
			// std::cout << fmt::format("Qubit {} : {} \n", node_index, path_literal);
		});
		this->foreach_cgate([&](auto const& node, auto node_index) {
			// If is a Z rotation save the current state of the qubit
			if (node.gate.is_z_rotation()) {
				auto qid = node.gate.target();
				auto map_element = pathsum_to_node_.find(qubit_state_[qid]);
				assert(map_element != pathsum_to_node_.end());
				node_to_pathsum_[node] = map_element;
				map_element->second.push_back(node_index);
			}
			if (node.gate.is(gate_set::pauli_x)) {
				auto target_qid = node.gate.target();
				auto search_it = qubit_state_[target_qid].find(1);
				if (search_it != qubit_state_[target_qid].end()) {
					qubit_state_[target_qid].erase(search_it);
				} else {
					qubit_state_[target_qid].emplace(1);
				}
				map_pathsum_to_node(target_qid, node, node_index);
			}
			if (node.gate.is(gate_set::cx)) {
				auto target_qid = node.gate.target();
				auto control_qid = node.gate.control();
				for (auto term : qubit_state_[control_qid]) {
					auto search_it = qubit_state_[target_qid].find(term);
					if (search_it != qubit_state_[target_qid].end()) {
						qubit_state_[target_qid].erase(search_it);
						continue;
					}
					qubit_state_[target_qid].emplace(term);
				}
				map_pathsum_to_node(target_qid, node, node_index);
			}
			// In case of hadamard gate a new path variable
			if (node.gate.is(gate_set::hadamard)) {
				auto qid = node.gate.target();
				qubit_state_[qid].clear();
				qubit_state_[qid].emplace((num_path_vars_++ << 1));
				map_pathsum_to_node(qid, node, node_index);
			}
		});
		this->foreach_coutput([&](auto& node, auto node_index) {
			node.gate.foreach_target([&](auto qid) {
				auto map_element = pathsum_to_node_.find(qubit_state_[qid]);
				assert(map_element != pathsum_to_node_.end());
				node_to_pathsum_[node] = map_element;
				map_element->second.push_back(node_index);
			});
		});
	}

private:
	std::unordered_map<esop_type, std::vector<uint32_t>> pathsum_to_node_;
	node_map<std::unordered_map<esop_type, std::vector<uint32_t>>::iterator, Network> node_to_pathsum_;
	uint32_t num_path_vars_;
	std::vector<esop_type> qubit_state_;
	std::vector<uint32_t> phy_virtual_map_;
};

} // namespace tweedledum
