/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../utils/device.hpp"
#include "../../views/mapping_view.hpp"
#include "../generic/reverse.hpp"
#include "sat_map.hpp"

#include <set>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `sabre_map`. */
struct sabre_map_params {
	uint32_t e_set_size = 200;
	float e_score_weight = .8;
	float decay_delta = 0.001;
	uint32_t num_rounds_decay_reset = 5;
	bool randomize_initial_map = false;
	bool use_look_ahead = true;
};

#pragma region Implementation details
namespace detail {

template<typename Network>
class sabre_mapper {
	using swap_type = std::tuple<uint32_t, uint32_t, double>;
public:
	sabre_mapper(device const& device, sabre_map_params const& params)
	    : device_(device)
	    , distances_(device.get_distance_matrix())
	    // Parameters
	    , e_set_size_(params.e_set_size)
	    , e_score_weight_(params.e_score_weight)
	    , decay_delta_(params.decay_delta)
	    , num_rounds_decay_reset_(params.num_rounds_decay_reset)
	    , randomize_initial_map_(params.randomize_initial_map)
	    , use_look_ahead_(params.use_look_ahead) 
	    // Temporary data
	    , qubits_decay_(device.num_vertices(), 1.0)
	{}

	mapping_view<Network> run(Network const& network, std::vector<uint32_t> initial_mapping = {})
	{
		mapping_view<Network> mapped_network(network, device_, false, randomize_initial_map_);
		if (!initial_mapping.empty()) {
			mapped_network.set_virtual_phy_map(initial_mapping);
		}

		network.clear_values();
		network.foreach_output([&](auto const& node) {
			network.foreach_child(node, [&](auto child_index) {
				auto& child = network.get_node(child_index);
				if (network.incr_value(child) == child.gate.num_io()) {
					front_layer_.push_back(child_index);
				}
			});
		});

		uint32_t num_swap_searches = 0;
		while (!front_layer_.empty()) {
			bool executed = false;
			std::vector<uint32_t> new_front_layer;
			std::set<uint32_t> phy_qubits;
			for (auto node_index : front_layer_) {
				auto& node = network.get_node(node_index);
				if (node.gate.is_meta()) {
					continue;
				} 
				// fmt::print("{}, {}\n", node_index, node.gate);
				// fmt::print("{}, {}\n", node.gate.control(), node.gate.target());
				if (node.gate.is_single_qubit()) {
					mapped_network.add_gate(node.gate, node.gate.target());
				} else if (!mapped_network.add_gate(node.gate, node.gate.control(), node.gate.target())) {
					new_front_layer.push_back(node_index);
					phy_qubits.emplace(mapped_network.virtual_phy_map(node.gate.control()));
					phy_qubits.emplace(mapped_network.virtual_phy_map(node.gate.target()));
					continue;
				}
				executed = true;
				network.foreach_child(node, [&](auto child_index) {
					auto& child = network.get_node(child_index);
					if (child.gate.is_meta()) {
						return;
					}
					if (network.incr_value(child) == child.gate.num_io()) {
						new_front_layer.push_back(child_index);
					}
				});
			}
			front_layer_ = new_front_layer;
			if (!executed) {
				auto [swap_q0, swap_q1, score] = find_swap(network, mapped_network, phy_qubits);
				num_swap_searches += 1;
				if (num_swap_searches % num_rounds_decay_reset_ == 0) {
					std::fill(qubits_decay_.begin(), qubits_decay_.end(), 1.0);
				} else {
					qubits_decay_.at(swap_q0) += decay_delta_;
					qubits_decay_.at(swap_q1) += decay_delta_;
				}
				mapped_network.add_swap(swap_q0, swap_q1);
			}
		}
		return mapped_network;
	}

private:
	swap_type find_swap(Network const& network,
	                    mapping_view<Network> const& mapped_network,
	                    std::set<uint32_t> const& phy_qubits)
	{
		// Obtain SWAP candidates
		std::vector<swap_type> swap_candidates;
		for (auto [v, u] : device_.edges) {
			auto search_v = phy_qubits.find(v);
			auto search_u = phy_qubits.find(u);
			if ((search_v != phy_qubits.end()) || (search_u != phy_qubits.end())) {
				swap_candidates.emplace_back(v, u, 0.0);
			}
		}

		if (use_look_ahead_) {
			select_e_layer(network);
		}

		for (auto& [swap_q0, swap_q1, score] : swap_candidates) {
			auto current_mapping = mapped_network.phy_virtual_map();
			std::swap(current_mapping[swap_q0], current_mapping[swap_q1]);
			double f_score = compute_score(network, mapped_network, current_mapping, front_layer_);
			score = f_score;

			if (!e_layer_.empty() && use_look_ahead_) {
				double max_decay = std::max(qubits_decay_[swap_q0], qubits_decay_[swap_q1]);
				double e_score = compute_score(network, mapped_network, current_mapping, e_layer_);
				f_score = f_score / front_layer_.size();
				e_score = e_score / e_layer_.size();
				score = max_decay * (f_score + (e_score_weight_ * e_score));
			}
		}
		std::sort(swap_candidates.begin(), swap_candidates.end(), [&](auto const& c0, auto const& c1) {
			return std::get<2>(c0) < std::get<2>(c1); 
		});
		return swap_candidates.front();
	}

	double compute_score(Network const& network, mapping_view<Network> const& mapped_network,
	                     std::vector<uint32_t> const& mapping, std::vector<uint32_t> const& gates)
	{
		double score = 0.0;
		for (auto node_index : gates) {
			auto& gate = network.get_node(node_index).gate;
			auto [q0, q1] = find_qubits(mapping,
			                            mapped_network.virtual_qubit(gate.control()),
			                            mapped_network.virtual_qubit(gate.target()));
			score += (distances_.at(q0).at(q1) - 1);
		}
		return score;
	}

	std::pair<uint32_t, uint32_t> find_qubits(std::vector<uint32_t> const& mapping, uint32_t q0,
	                                          uint32_t q1)
	{
		uint32_t position_q0 = std::distance(mapping.begin(),
		                                     std::find(mapping.begin(), mapping.end(), q0));
		uint32_t position_q1 = std::distance(mapping.begin(),
		                                     std::find(mapping.begin(), mapping.end(), q1));
		assert(position_q0 < mapping.size());
		assert(position_q1 < mapping.size());
		return {position_q0, position_q1};
	}

	// 'e_layer' is extented front layer, i.e. the look-ahead
	void select_e_layer(Network const& network)
	{
		e_layer_.clear();
		std::vector<uint32_t> tmp_incremented;
		std::vector<uint32_t> tmp_e_layer = front_layer_;
		while (!tmp_e_layer.empty()) {
			std::vector<uint32_t> tmp = {};
			for (auto node_index : tmp_e_layer) {
				auto& node = network.get_node(node_index);
				network.foreach_child(node, [&](auto child_index) {
					auto& child = network.get_node(child_index);
					if (child.gate.is_meta()) {
						return;
					}
					tmp_incremented.emplace_back(child_index);
					assert(network.value(child) < child.gate.num_io());
					if (network.incr_value(child) == child.gate.num_io()) {
						tmp.emplace_back(child_index);
						if (child.gate.is_single_qubit()) {
							return;
						}
						e_layer_.emplace_back(child_index);
					}
				});
				if (e_layer_.size() >= e_set_size_) {
					goto undo_increment;
				}
			}
			tmp_e_layer = tmp;
		}
	undo_increment:
		for (auto node_index : tmp_incremented) {
			auto& node = network.get_node(node_index);
			network.decr_value(node); 
		} 
	}

private:
	// Problem data
	device const& device_;
	std::vector<std::vector<uint32_t>> distances_;

	// Algorithm parameters
	uint32_t e_set_size_;
	float e_score_weight_;
	float decay_delta_;
	uint32_t num_rounds_decay_reset_;
	bool randomize_initial_map_;
	bool use_look_ahead_;
	// Algorithm temporary data
	std::vector<uint32_t> front_layer_;
	std::vector<uint32_t> e_layer_;
	std::vector<float> qubits_decay_;
};

} // namespace detail
#pragma endregion

/*! \brief SABRE-base mappper
 *
   \verbatim embed:rst

   Mapper based on the SABRE algorithm :cite:`SABRE19`.

   \endverbatim
 * 
 * \algtype mapping
 * \algexpects A network
 * \algreturns A mapped network
 */
template<typename Network>
mapping_view<Network> sabre_map(Network const& network, device const& device,
                                sabre_map_params const& params = {})
{
	detail::sabre_mapper<Network> mapper(device, params);
	// Heuristic v0
	auto init = sat_initial_map(network, device);
	mapping_view<Network> h0_ntk = mapper.run(network, init);
	h0_ntk = mapper.run(reverse(static_cast<Network>(network)), h0_ntk.virtual_phy_map());

	// Heuristic v1
	mapping_view<Network> h1_ntk = mapper.run(reverse(static_cast<Network>(network)), init);
	h1_ntk = mapper.run((network), h1_ntk.virtual_phy_map());
	h1_ntk = mapper.run(reverse(static_cast<Network>(network)), h1_ntk.virtual_phy_map());

	if (h0_ntk.num_gates() < h1_ntk.num_gates()) {
		return h0_ntk;
	}
	return h1_ntk;
}

} // namespace tweedledum
