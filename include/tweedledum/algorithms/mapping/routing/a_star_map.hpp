/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"
#include "../../target/device.hpp"
#include "../../views/layers_view.hpp"

#include <algorithm>
#include <bill/dd/zdd.hpp>
#include <cassert>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `a_star_map`. */
struct a_star_map_params {
	bool randomize_initial_map = true;
	bool use_look_ahead = true;
};

#pragma region Implementation details
namespace detail {

template<typename Network>
class a_star_mapper {
	using swap_type = std::tuple<uint32_t, uint32_t>;
	using op_type = typename Network::op_type;

public:
	a_star_mapper(device const& device, a_star_map_params const& params)
	    : device_(device)
	    , distances_(device.distance_matrix())
	    // Parameters
	    , randomize_initial_map_(params.randomize_initial_map)
	    , use_look_ahead_(params.use_look_ahead) 
	{
		compute_swap_layers();
	}

	mapped_dag run(Network const& original, std::vector<uint32_t> initial_mapping = {})
	{
		mapped_dag mapped(original, device_);
		layers_view<Network> layered(original);

		// Define mapping first layer
		// TODO: make this better
		if (initial_mapping.empty()) {
			initial_mapping = compute_initial_mapping(layered, mapped);
		}
		mapped.v_to_phy(initial_mapping);

		// Map all layers
		for (uint32_t layer = 1u; layer < layered.num_layers(); ++layer) {
			std::set<wire_id> v_qubits;

			execute_layer(layered, layer, mapped, v_qubits);
			if (!unexecuted_nodes_.empty()) {
				fix_layer(layered, mapped, v_qubits);
				unexecuted_nodes_.clear();
			}
		}
		return mapped;
	}

//
private:
	bool execute_layer(Network const& layered, uint32_t layer, mapped_dag& mapped,
	                   std::set<wire_id>& v_qubits)
	{
		std::vector<node_id> nodes = layered.layer(layer);
		for (node_id const n_id : nodes) {
			op_type const& op = layered.node(n_id).op;
			if (op.is_one_qubit()) {
				mapped.create_op(op, op.target());
			} else if (mapped.add_op(op, op.control(), op.target()) == node::invalid_id) {
				unexecuted_nodes_.push_back(n_id);
				v_qubits.emplace(mapped.wire_to_v(op.control()));
				v_qubits.emplace(mapped.wire_to_v(op.target()));
				continue;
			}
		}
	}

// A* search
private:
	struct node {
		std::vector<wire_id> mapping;
		uint32_t swap;
		uint32_t previous;
		uint32_t g;
		uint32_t h;
		bool closed;

		node(std::vector<wire_id> const& mapping_, uint32_t edge, uint32_t prev, uint32_t g,
		     uint32_t h)
		    : mapping(mapping_)
		    , swap(edge)
		    , previous(prev)
		    , g(g)
		    , h(h)
		    , closed(false)
		{}
	};

	void fix_layer(layers_view<Network> const& layered, mapped_dag& mapped,
	               std::set<uint32_t> const& v_qubits)
	{
	fix_layer_again:
		std::vector<node> nodes;
		std::vector<uint32_t> open_nodes;
		std::vector<uint32_t> closed_nodes;
		std::unordered_map<std::vector<uint32_t>, uint32_t> mappings;

		// Initial node
		nodes.emplace_back(mapped.phy_to_v(), 0, 0, 0, 0);
		open_nodes.emplace_back(0);
		mappings.emplace(mapped.phy_to_v(), 0);

		// Obtain SWAP candidates
		while (!open_nodes.empty()) {
			node node = nodes.at(open_nodes.back());
			closed_nodes.push_back(open_nodes.back());
			nodes.at(open_nodes.back()).closed = true;
			open_nodes.pop_back();

			for (uint32_t i = 0u; i < device_.edges.size(); ++i) {
				// Do not apply the same SWAP gate twice in a row
				// if (i == node.swap) {
				// 	continue;
				// }
				auto [v, u] =  device_.edges(i);
				auto search_v = v_qubits.find(node.mapping.at(v));
				auto search_u = v_qubits.find(node.mapping.at(u));
				// Apply only SWAP operations including at least one qubit in used_qubits 
				if ((search_v == v_qubits.end()) && (search_u == v_qubits.end())) {
					continue;
				}
				std::vector<wire_id> new_mapping = node.mapping;
				std::swap(new_mapping.at(v), new_mapping.at(u));

				// Check if mapping was already considered
				auto [map_it, was_added] = mappings.emplace(new_mapping, nodes.size());
				auto& new_node = was_added ? nodes.emplace_back(node) :
						     nodes.at(map_it->second);
				if (was_added) {
					open_nodes.push_back(nodes.size() - 1);
					std::swap(new_node.mapping[v], new_node.mapping[u]);
				} else if (new_node.closed) {
					continue;
				}
				// Update new node info
				new_node.previous = closed_nodes.back();
				new_node.swap = i;
				new_node.g = node.g + 1;
				new_node.h = 0;

				// Check wheter a goal state is reached (i.e. whether any gate can be applied)
				// and determine heuristic cost
				auto min_it = open_nodes.end();
				for (node_id const n_id: unexecuted_nodes_) {
					op_type const& op = layered_network.node(n_id).op;
					auto [q0, q1] = find_qubits(node.mapping,
					                            mapped.wire_to_v(op.control()),
					                            mapped.wire_to_v(op.target()));
					if (distances_.at(q0).at(q1) == 1) {
						min_it = open_nodes.end() - 1;
					}
					new_node.h += distances_.at(q0).at(q1);
				}
				if (min_it != open_nodes.end()) {
					closed_nodes.push_back(open_nodes.back());
					goto found_goal;
				}
				min_it = std::min_element(open_nodes.begin(), open_nodes.end(),
				                          [&](auto a_idx, auto b_idx) {
					                          auto& a = nodes[a_idx];
					                          auto& b = nodes[b_idx];
					                          return (a.g + a.h) < (b.g + b.h);
				                          });
				std::swap(*min_it, open_nodes.back());
			}
		}

	found_goal:
		std::vector<std::pair<uint32_t, uint32_t>> swaps;
		auto& node = nodes.at(closed_nodes.back());
		while (node.previous) {
			swaps.emplace_back(device_.edges[node.swap]);
			node = nodes.at(node.previous);
		}
		swaps.push_back(device_.edges[node.swap]);
		std::reverse(swaps.begin(), swaps.end());
		for (auto [x, y] : swaps) {
			mapped_network.add_swap(x, y);
		}

		std::vector<uint32_t> tmp_layer;
		std::set<uint32_t> tmp_virtual_qubits;
		for (auto node_index : layer) {
			auto& gate = layered_network.get_node(node_index).gate;
			if (!mapped_network.add_gate(gate, gate.control(), gate.target())) {
				tmp_layer.push_back(node_index);
				tmp_virtual_qubits.emplace(mapped_network.virtual_qubit(gate.control()));
				tmp_virtual_qubits.emplace(mapped_network.virtual_qubit(gate.target()));
			}
		}
		layer = tmp_layer;
		virtual_qubits = tmp_virtual_qubits;
		if (!layer.empty()) {
			goto fix_layer_again;
		}
	}

	std::pair<uint32_t, uint32_t> find_qubits(std::vector<uint32_t> const& mapping, uint32_t q0,
	                                          uint32_t q1)
	{
		uint32_t position_q0 = std::distance(mapping.begin(),
		                                     std::find(mapping.begin(), mapping.end(), q0));
		uint32_t position_q1 = std::distance(mapping.begin(),
		                                     std::find(mapping.begin(), mapping.end(), q1));
		return {position_q0, position_q1};
	}

	std::vector<uint32_t> compute_initial_mapping(layers_view<Network> const& layered_network,
	                                              mapping_view<Network> const& mapped_network)
	{
		constexpr uint32_t invalid = std::numeric_limits<uint32_t>::max();
		std::vector<uint32_t> mapping(device_.num_nodes, invalid);
		std::vector<uint32_t> unused_phy_qubits(device_.num_nodes);
		std::iota(unused_phy_qubits.begin(), unused_phy_qubits.end(), 0u);
		std::vector<uint32_t> layer = layered_network.layer(1);
		auto max = std::max_element(swap_layers_.begin(), swap_layers_.end(),
		                            [](auto const& a, auto const& b) { return a.size() < b.size(); });
		std::vector<uint32_t> edges = *max;
		if (1) {
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(edges.begin(), edges.end(), g);
		}
		fmt::print("Edges: {}\n", fmt::join(edges.begin(), edges.end(), ","));
		fmt::print("\n", fmt::join(edges.begin(), edges.end(), ","));
		for (auto node_index : layer) {
			auto& gate = layered_network.get_node(node_index).gate;
			if (gate.is_single_qubit()) {
				continue;
			}
			auto [phy_q0, phy_q1] = device_.edges[edges.back()];
			fmt::print("{}, {}\n", phy_q0, phy_q1);
			mapping.at(mapped_network.virtual_qubit(gate.control())) = phy_q0;
			mapping.at(mapped_network.virtual_qubit(gate.target())) = phy_q1;
			unused_phy_qubits.erase(std::remove_if(unused_phy_qubits.begin(), unused_phy_qubits.end(), [&](auto a) {
				return a == phy_q0 || a == phy_q1;
			}), unused_phy_qubits.end());
			// fmt::print("Unused: {}\n", fmt::join(unused_phy_qubits.begin(), unused_phy_qubits.end(), ","));
			edges.pop_back();
		}
		for (auto& element : mapping) {
			if (element == invalid) {
				element = unused_phy_qubits.back();
				unused_phy_qubits.pop_back();
			}
		}
		fmt::print("Unused: {}\n", fmt::join(mapping.begin(), mapping.end(), ","));
		return mapping;
	}

	void compute_swap_layers()
	{
		zdd_base zdd_swap_layers(device_.edges.size());
		zdd_swap_layers.build_tautologies();

		auto univ_fam = zdd_swap_layers.tautology();
		std::vector<zdd_base::node> edges_p;

		std::vector<std::vector<uint8_t>> incidents(device_.num_nodes);
		for (auto i = 0u; i < device_.edges.size(); ++i) {
			incidents[device_.edges[i].first].push_back(i);
			incidents[device_.edges[i].second].push_back(i);
		}

		for (auto& i : incidents) {
			std::sort(i.rbegin(), i.rend());
			auto set = zdd_swap_layers.bot();
			for (auto o : i) {
				set = zdd_swap_layers.union_(set, zdd_swap_layers.elementary(o));
			}
			edges_p.push_back(set);
		}

		auto edges_union = zdd_swap_layers.bot();
		for (int v = device_.num_nodes - 1; v >= 0; --v) {
			edges_union = zdd_swap_layers.union_(edges_union, zdd_swap_layers.choose(edges_p[v], 2));
		}
		auto layers = zdd_swap_layers.nonsupersets(univ_fam, edges_union);
		zdd_swap_layers.sets_to_vector(layers, swap_layers_);
	}

private:
	// Problem data
	device const& device_;
	std::vector<std::vector<uint32_t>> distances_;

	// Algorithm parameters
	bool randomize_initial_map_;
	bool use_look_ahead_;

	// Algorithm temporary data
	std::vector<std::vector<uint32_t>> swap_layers_;
	std::vector<node_id> unexecuted_nodes_;

};

} // namespace detail
#pragma endregion

/*! \brief 
 *
   \verbatim embed:rst

   Mapper based on the .

   \endverbatim
 * 
 * \algtype mapping
 * \algexpects A network
 * \algreturns A mapped network
 */
template<typename Network>
mapping_view<Network> a_star_map(Network const& network, device const& device,
                                a_star_map_params const& params = {})
{
	detail::a_star_mapper<Network> mapper(device, params);
	mapping_view<Network> mapped_ntk = mapper.run(network);
	return mapped_ntk;
}

} // namespace tweedledum
