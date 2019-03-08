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

/*! \brief
 *
 * This view computes the path sums of each node of the network. It implements the
 * network interface methods `get_pathsum`. The pathsums are computed at construction
 * and can be recomputed by calling the `update` method.
 *
 * **Required gate functions:**
 *
 * **Required network functions:**
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
	{
		update();
	}

	/*! \brief Returns the path equations of a node. */
	auto& get_pathsum(node_type const& node) const
	{
		return node_to_pathsum_[node]->first;
	}

	void update()
	{
		compute_pathsums();
	}

private:
	void compute_pathsums()
	{
		std::vector<esop_type> qubit_state(this->num_qubits());

		// Initialize qubit_state with initial path literals
		this->foreach_cinput([&](auto& node, auto node_index) {
			const auto path_literal = ((node_index + 1) << 1);
			qubit_state[node_index].emplace(path_literal);
			auto map_element = pathsum_to_node_.emplace(std::make_pair(qubit_state[node_index], std::vector<uint32_t>{node_index}));
			node_to_pathsum_[node] = map_element.first;
			// std::cout << fmt::format("Qubit {} : {}\n", node_index, path_literal);
		});

		this->foreach_cgate([&](auto const& node, auto node_index) {
			// If is a Z rotation save the current state of the qubit
			if (node.gate.is_z_rotation()) {
				node.gate.foreach_target([&](auto qid) {
					// std::cout << fmt::format("Adding Rz: {} \n", qid);
					auto map_element = pathsum_to_node_.find(qubit_state[qid]);
					assert(map_element != pathsum_to_node_.end());
					node_to_pathsum_[node] = map_element;
					map_element->second.push_back(node_index);
				});
			}
			if (node.gate.is(gate_set::pauli_x)) {
				node.gate.foreach_target([&](auto target_qid) {
					// std::cout << fmt::format("X {}\n", target_qid);
					auto search_it = qubit_state[target_qid].find(1);
					if (search_it != qubit_state[target_qid].end()) {
							qubit_state[target_qid].erase(search_it);
					} else {
						qubit_state[target_qid].emplace(1);
					}
					auto map_element = pathsum_to_node_.emplace(std::make_pair(qubit_state[target_qid], std::vector<uint32_t>{node_index}));
						if (map_element.second == false) {
							map_element.first->second.push_back(node_index);
						}
					node_to_pathsum_[node] = map_element.first;
				});
			}
			if (node.gate.is(gate_set::cx)) {
				// std::cout << "cx\n";
				node.gate.foreach_target([&](auto target_qid) {
					// std::cout << fmt::format("{}", target_qid);
					node.gate.foreach_control([&](auto control_qid) {
						// std::cout << fmt::format(" : {}\n", control_qid);
						for (auto term : qubit_state[control_qid]) {
							auto search_it = qubit_state[target_qid].find(term);
							if (search_it != qubit_state[target_qid].end()) {
								qubit_state[target_qid].erase(search_it);
								continue;
							}
							qubit_state[target_qid].emplace(term);
						}
						auto map_element = pathsum_to_node_.emplace(std::make_pair(qubit_state[target_qid], std::vector<uint32_t>{node_index}));
						if (map_element.second == false) {
							map_element.first->second.push_back(node_index);
						}
						node_to_pathsum_[node] = map_element.first;
					});
				});
			}
			// In case of hadamard gate a new path variable
			if (node.gate.is(gate_set::hadamard)) {
				node.gate.foreach_target([&](auto qid) {
					// std::cout << "New path variable\n";
					qubit_state[qid].clear();
					qubit_state[qid].emplace((num_path_vars_++ << 1));
					auto map_element = pathsum_to_node_.emplace(std::make_pair(qubit_state[qid], std::vector<uint32_t>{node_index}));
					node_to_pathsum_[node] = map_element.first;
				});
			}
		});
		this->foreach_coutput([&](auto& node, auto node_index) {
			node.gate.foreach_target([&](auto qid) {
				auto map_element = pathsum_to_node_.find(qubit_state[qid]);
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
};

} // namespace tweedledum
