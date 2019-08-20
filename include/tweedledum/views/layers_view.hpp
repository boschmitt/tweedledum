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

	explicit layers_view(Network const& network)
	    : immutable_view<Network>(network)
	    , node_layer_(network)
	{
		if (this->num_io()) {
			update();
		}
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

	void compute_layers()
	{
		this->foreach_input([&](auto const& node) {
			layer_nodes_.at(0).push_back(this->index(node));
			node_layer_[node] = 0u;
		});
		this->foreach_gate([&](auto const& node, auto node_index) {
			uint32_t layer = 0u;
			this->foreach_child(node, [&](auto child_index) {
				auto& child = this->get_node(child_index);
				layer = std::max(layer, node_layer_[child]);
			});
			layer += 1;
			if (layer == layer_nodes_.size()) {
				layer_nodes_.emplace_back();
			}
			node_layer_[node] = layer;
			layer_nodes_.at(layer).push_back(node_index);
		});
		layer_nodes_.emplace_back();
		this->foreach_output([&](auto const& node) {
			layer_nodes_.back().push_back(this->index(node));
			node_layer_[node] = layer_nodes_.size() - 1;
		});
	}

private:
	node_map<uint32_t, Network> node_layer_;
	std::vector<std::vector<uint32_t>> layer_nodes_;
};

} // namespace tweedledum
