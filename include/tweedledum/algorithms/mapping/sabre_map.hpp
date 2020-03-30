/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"
#include "../../utils/device.hpp"
#include "../transformations/reverse.hpp"
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
	using swap_type = std::tuple<wire_id, wire_id, double>;
	using node_type = typename Network::node_type;
	using op_type = typename Network::op_type;

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

	mapped_dag run(Network const& original, std::vector<wire_id> initial_mapping = {})
	{
		mapped_dag mapped(original, device_);
		if (!initial_mapping.empty()) {
			mapped.v_to_phy(initial_mapping);
		}

		original.clear_values();
		original.foreach_output([&](node_type const& node, node_id const id) {
			if (node.op.is_meta()) {
				return;
			}
			if (original.incr_value(node) == node.op.num_wires()) {
				front_layer_.push_back(id);
			}
		});

		uint32_t num_swap_searches = 0;
		while (!front_layer_.empty()) {
			std::set<wire_id> phy_qubits;
			if (!execute_front_layer(original, mapped, phy_qubits)) {
				auto [swap_q0, swap_q1, score] = find_swap(original, mapped, phy_qubits);
				num_swap_searches += 1;
				if (num_swap_searches % num_rounds_decay_reset_ == 0) {
					std::fill(qubits_decay_.begin(), qubits_decay_.end(), 1.0);
				} else {
					qubits_decay_.at(swap_q0) += decay_delta_;
					qubits_decay_.at(swap_q1) += decay_delta_;
				}
				mapped.create_swap(swap_q0, swap_q1);
			}
		}
		return mapped;
	}

private:
	bool execute_front_layer(Network const& original, mapped_dag& mapped,
	                         std::set<wire_id>& phy_qubits)
	{
		bool executed = false;
		std::vector<node_id> new_front_layer;
		for (node_id n_id : front_layer_) {
			node_type const& node = original.node(n_id);
			op_type const& op = node.op;
			if (op.is_meta()) {
				continue;
			} 
			if (op.is_one_qubit()) {
				mapped.add_op(op, op.target());
			} else if (mapped.add_op(op, op.control(), op.target()) == node::invalid) {
				new_front_layer.push_back(n_id);
				phy_qubits.emplace(mapped.wire_to_phy(op.control()));
				phy_qubits.emplace(mapped.wire_to_phy(op.target()));
				continue;
			}
			executed = true;
			original.foreach_child(node, [&](node_type const& child, node_id child_id) {
				if (child.op.is_meta()) {
					return;
				}
				if (original.incr_value(child) == child.op.num_wires()) {
					new_front_layer.push_back(child_id);
				}
			});
		}
		front_layer_ = new_front_layer;
		return executed;
	}

	swap_type find_swap(Network const& original, mapped_dag const& mapped,
	                    std::set<wire_id> const& phy_qubits)
	{
		// Obtain SWAP candidates
		std::vector<swap_type> swap_candidates;
		for (auto [w, u] : device_.edges) {
			wire_id w_phy(w, true);
			wire_id u_phy(u, true);
			auto search_v = phy_qubits.find(w_phy);
			auto search_u = phy_qubits.find(u_phy);
			if ((search_v != phy_qubits.end()) || (search_u != phy_qubits.end())) {
				swap_candidates.emplace_back(w_phy, u_phy, 0.0);
			}
		}

		if (use_look_ahead_) {
			select_e_layer(original);
		}

		for (auto& [swap_q0, swap_q1, score] : swap_candidates) {
			auto current_mapping = mapped.phy_to_v();
			std::swap(current_mapping[swap_q0], current_mapping[swap_q1]);
			double f_score = compute_score(original, mapped, current_mapping, front_layer_);
			score = f_score;

			if (!e_layer_.empty() && use_look_ahead_) {
				double max_decay = std::max(qubits_decay_[swap_q0], qubits_decay_[swap_q1]);
				double e_score = compute_score(original, mapped, current_mapping, e_layer_);
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

	double compute_score(Network const& original, mapped_dag const& mapped,
	                     std::vector<wire_id> const& mapping, std::vector<node_id> const& gates)
	{
		double score = 0.0;
		for (node_id n_id : gates) {
			op_type const& op = original.node(n_id).op;
			auto [q0, q1] = v_to_pos(mapping, mapped.wire_to_v(op.control()),
			                         mapped.wire_to_v(op.target()));
			score += (distances_.at(q0).at(q1) - 1);
		}
		return score;
	}

	std::pair<uint32_t, uint32_t> v_to_pos(std::vector<wire_id> const& phy_to_v, wire_id v0,
	                                       wire_id v1)
	{
		uint32_t pos_v0;
		uint32_t pos_v1;
		for (uint32_t i = 0, found = 0u; i < phy_to_v.size() && found != 2; ++i) {
			if (phy_to_v.at(i) == v0) {
				pos_v0 = i;
				++found;
			} else if (phy_to_v.at(i) == v1) {
				pos_v1 = i;
				++found;
			}
		}
		return {pos_v0, pos_v1};
	}

	// 'e_layer' is extented front layer, i.e. the look-ahead
	void select_e_layer(Network const& original)
	{
		e_layer_.clear();
		std::vector<node_id> tmp_incremented;
		std::vector<node_id> tmp_e_layer = front_layer_;
		while (!tmp_e_layer.empty()) {
			std::vector<node_id> tmp = {};
			for (node_id n_id : tmp_e_layer) {
				node_type const& node = original.node(n_id);
				original.foreach_child(node, [&](node_type const& child, node_id child_id) {
					if (child.op.is_meta()) {
						return;
					}
					tmp_incremented.emplace_back(child_id);
					assert(original.value(child) < child.op.num_wires());
					if (original.incr_value(child) == child.op.num_wires()) {
						tmp.emplace_back(child_id);
						if (!child.op.is_two_qubit()) {
							return;
						}
						e_layer_.emplace_back(child_id);
					}
				});
				if (e_layer_.size() >= e_set_size_) {
					goto undo_increment;
				}
			}
			tmp_e_layer = tmp;
		}
	undo_increment:
		for (node_id n_id : tmp_incremented) {
			node_type const& node = original.node(n_id);
			original.decr_value(node); 
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
	std::vector<node_id> front_layer_;
	std::vector<node_id> e_layer_;
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
 * \algexpects A original
 * \algreturns A mapped original
 */
template<typename Network>
mapped_dag sabre_map(Network const& original, device const& device,
                     sabre_map_params const& params = {})
{
	detail::sabre_mapper<Network> mapper(device, params);
	// Heuristic v0
	auto init = sat_initial_map(original, device);
	mapped_dag h0_ntk = mapper.run(original, init);
	h0_ntk = mapper.run(reverse(static_cast<Network>(original)), h0_ntk.v_to_phy());

	// Heuristic v1
	mapped_dag h1_ntk = mapper.run(reverse(static_cast<Network>(original)), init);
	h1_ntk = mapper.run((original), h1_ntk.v_to_phy());
	h1_ntk = mapper.run(reverse(static_cast<Network>(original)), h1_ntk.v_to_phy());

	if (h0_ntk.num_operations() < h1_ntk.num_operations()) {
		return h0_ntk;
	}
	return h1_ntk;
}

} // namespace tweedledum
