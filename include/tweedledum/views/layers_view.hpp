/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../traits.hpp"
#include "../utils/node_map.hpp"
#include "immutable_view.hpp"

#include <vector>

namespace tweedledum {

/*! \brief Implements the network interface methods `layer(node)`, `layer(layer_index)` and `depth`.
 *
 * The layers are computed at construction and can be recomputed by calling the `update` method.
 * 
 * NOTE: The 0th and the last layers correspond to the input and output nodes, respectively.
 */
template<typename Network>
class layers_view : public immutable_view<Network> {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using link_type = typename Network::link_type;
	using storage_type = typename Network::storage_type;

	explicit layers_view(Network& network)
	    : immutable_view<Network>(network)
	    , node_layer_(network)
	{
		update();
	}

	// NOTE: the depth of a quantum circuit is the number layers with gates.
	uint32_t depth() const
	{
		// Since the adition of a qubit (or cbit) adds an input and an output node to the
		// network, the number of layers must never be 1.
		assert(layer_nodes_.size() == 0 || layer_nodes_.size() >= 2);
		return layer_nodes_.empty() ? 0u : layer_nodes_.size() - 2;
	}

	uint32_t num_layers() const
	{
		return layer_nodes_.size();
	}

	uint32_t layer(node_type const& node) const
	{
		return node_layer_[node];
	}

	std::vector<uint32_t> layer(uint32_t index) const
	{
		return layer_nodes_[index];
	}

	void update()
	{
		reset();
		compute_layers();
		this->clear_values();
	}

private:
	void reset()
	{
		node_layer_.reset(0);
		layer_nodes_.clear();
		if (this->num_io() > 0) {
			layer_nodes_.resize(1);
		}
		this->clear_values();
	}

	uint32_t compute_layers(node_type const& node)
	{
		if (this->value(node)) {
			return node_layer_[node];
		}

		if (node.gate.is_one_of(gate_lib::input)) {
			layer_nodes_.at(0).push_back(this->index(node));
			return node_layer_[node] = 0u;
		}

		uint32_t level = 0u;
		this->foreach_child(node, [&](auto child_index) {
			level = std::max(level, compute_layers(this->node(child_index)));
		});
		level += 1;
		if (level == layer_nodes_.size()) { 
			layer_nodes_.emplace_back();
		}
		layer_nodes_.at(level).push_back(this->index(node));
		this->set_value(node, 1u);
		return node_layer_[node] = level;
	}

	void compute_layers()
	{
		this->foreach_output([&](auto const& node) {
			compute_layers(node);
		});
	}

private:
	node_map<uint32_t, Network> node_layer_;
	std::vector<std::vector<uint32_t>> layer_nodes_;
};

} // namespace tweedledum
