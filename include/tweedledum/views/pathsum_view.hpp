/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../traits.hpp"
#include "../utils/hash.hpp"
#include "../utils/node_map.hpp"
#include "immutable_view.hpp"

#include <array>
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
 * construction. The network must be on basis {SWAP, CX, Rx, Rz, H}
 */
template<typename Network>
class pathsum_view : public immutable_view<Network> {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using link_type = typename Network::link_type;
	using storage_type = typename Network::storage_type;

	using esop_type = std::set<uint32_t>;

	explicit pathsum_view(Network& network, bool ignore_single_qubit = false)
	    : immutable_view<Network>(network)
	    , pathsum_to_node_()
	    , node_to_pathsum_(network)
	    , num_path_vars_(network.num_io() + 1)
	    , qubit_state_(network.num_io())
	    , phy_virtual_map_(network.num_qubits(), 0)
	    , ignore_single_qubit_(ignore_single_qubit)
	{
		// This seems a bit ridiculous
		// TODO: come up with a better way to check for allowed gates
		assert(network.check_gate_set(gate_lib::cx, gate_lib::swap, gate_lib::rx,
		                              gate_lib::rz, gate_lib::hadamard));
		std::iota(phy_virtual_map_.begin(), phy_virtual_map_.end(), 0);
		network.foreach_qubit([&](io_id id) {
			virtual_id_map_.emplace_back(id);
		});
		compute_pathsums();
	}

	// Pathsums can be employed to verify mappings.  Assuming that the mapping do not screw-up
	// adding single qubit gates, we can safely ignore them and verify if the set of output path sums
	// of the original circuit mataches the set of output path sums of the mapped circuit.
	//
	// The user need to pass the _initial_ virtual->physical mapping so that the path literals
	// can be placed correctly.
	explicit pathsum_view(Network& network, std::vector<uint32_t> const& virtual_phy_map,
	                      bool ignore_single_qubit = false)
	    : immutable_view<Network>(network)
	    , pathsum_to_node_()
	    , node_to_pathsum_(network)
	    , num_path_vars_(network.num_io() + 1)
	    , qubit_state_(network.num_io())
	    , phy_virtual_map_(network.num_qubits(), 0)
	    , ignore_single_qubit_(ignore_single_qubit)
	{
		assert(network.check_gate_set(gate_lib::cx, gate_lib::swap, gate_lib::rx,
		                              gate_lib::rz, gate_lib::hadamard));
		for (uint32_t i = 0; i < virtual_phy_map.size(); ++i) {
			phy_virtual_map_[virtual_phy_map[i]] = i;
		}
		network.foreach_qubit([&](io_id id) {
			virtual_id_map_.push_back(id);
		});
		compute_pathsums();
	}

	/*! \brief Returns the path equations of a node. */
	auto& get_pathsum(node_type const& node) const
	{
		return node_to_pathsum_[node]->first;
	}

private:
	void map_pathsum_to_node(io_id qid, node_type const& node, uint32_t node_index)
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
		uint32_t i = 0;
		this->foreach_input([&](auto& node, auto node_index) {
			const auto id = node.gate.target();
			uint32_t path_literal;
			if (id.is_qubit()) {
				path_literal = (((virtual_id_map_.at(phy_virtual_map_.at(i++))) + 1) << 1);
			} else {
				path_literal = ((id.index() + 1) << 1);
			}
			qubit_state_.at(id.index()).emplace(path_literal);
			map_pathsum_to_node(id, node, node_index);
		});
		this->foreach_gate([&](auto const& node, auto node_index) {
			auto const& gate = node.gate;
			// If is a Z rotation save the current state of the qubit
			if (gate.is_z_rotation() && !ignore_single_qubit_) {
				auto qid = gate.target();
				auto map_element = pathsum_to_node_.find(qubit_state_[qid]);
				assert(map_element != pathsum_to_node_.end());
				node_to_pathsum_[node] = map_element;
				map_element->second.push_back(node_index);
			} else if (gate.is(gate_lib::rx) && !ignore_single_qubit_) {
				if (gate.rotation_angle() != angles::pi) {
					auto qid = gate.target();
					qubit_state_[qid].clear();
					qubit_state_[qid].emplace((num_path_vars_++ << 1));
					map_pathsum_to_node(qid, node, node_index);
					return;
				}
				auto target_qid = gate.target();
				auto search_it = qubit_state_[target_qid].find(1);
				if (search_it != qubit_state_[target_qid].end()) {
					qubit_state_[target_qid].erase(search_it);
				} else {
					qubit_state_[target_qid].emplace(1);
				}
				map_pathsum_to_node(target_qid, node, node_index);
			} else if (gate.is(gate_lib::cx)) {
				auto target_qid = gate.target();
				auto control_qid = gate.control();
				for (auto term : qubit_state_[control_qid]) {
					auto search_it = qubit_state_[target_qid].find(term);
					if (search_it != qubit_state_[target_qid].end()) {
						qubit_state_[target_qid].erase(search_it);
						continue;
					}
					qubit_state_[target_qid].emplace(term);
				}
				map_pathsum_to_node(target_qid, node, node_index);
			} else if (gate.is(gate_lib::swap)) {
				std::array<io_id, 2> targets = {io_invalid, io_invalid};
				uint32_t i = 0;
				gate.foreach_target([&](auto qid) {
					targets[i++] = qid;
				});
				std::swap(qubit_state_[targets[0]], qubit_state_[targets[1]]);
			} else if (gate.is(gate_lib::hadamard) && !ignore_single_qubit_) {
				// In case of hadamard gate a new path variable
				auto qid = gate.target();
				qubit_state_[qid].clear();
				qubit_state_[qid].emplace((num_path_vars_++ << 1));
				map_pathsum_to_node(qid, node, node_index);
			}
		});
		this->foreach_output([&](auto& node, auto node_index) {
			node.gate.foreach_target([&](auto qid) {
				auto map_element = pathsum_to_node_.find(qubit_state_[qid]);
				assert(map_element != pathsum_to_node_.end());
				node_to_pathsum_[node] = map_element;
				map_element->second.push_back(node_index);
			});
		});
	}

private:
	using esop_hash_type = std::unordered_map<esop_type, std::vector<uint32_t>, hash<esop_type>>;

	esop_hash_type pathsum_to_node_;
	node_map<typename esop_hash_type::iterator, Network> node_to_pathsum_;
	uint32_t num_path_vars_;
	std::vector<esop_type> qubit_state_;
	std::vector<uint32_t> phy_virtual_map_;
	std::vector<io_id> virtual_id_map_;
	bool ignore_single_qubit_;
};

} // namespace tweedledum
