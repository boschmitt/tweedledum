/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../views/layers_view.hpp"
#include "shallow_duplicate.hpp"

#include <cassert>

namespace tweedledum {

/*! \brief Generic function to reverse a network.
 *
 * This function requires a template parameter that cannot be inferred. Useful when duplicate into a
 * different network format. Gate type must be the same though.
*/
template<class NewNetwork, class Network>
NewNetwork rewrite_network(Network const& network)
{
	static_assert(std::is_same_v<typename Network::gate_type, typename NewNetwork::gate_type>);

	NewNetwork result = shallow_duplicate<NewNetwork>(network);
	layers_view<Network> layers(network);
	for (uint32_t i = layers.num_layers() - 2; i > 0; --i) {
		std::vector<uint32_t> layer = layers.layer(i);
		for (auto node_index : layer) {
			auto& node = layers.get_node(node_index);
			result.emplace_gate(node.gate);
		}
	}
	return result;
}

template<class Network>
Network reverse(Network const& network)
{
	Network result = shallow_duplicate(network);
	layers_view<Network> layers(network);
	for (uint32_t i = layers.num_layers() - 2; i > 0; --i) {
		std::vector<uint32_t> layer = layers.layer(i);
		for (auto node_index : layer) {
			auto& node = layers.get_node(node_index);
			result.emplace_gate(node.gate);
		}
	}
	return result;
}

} // namespace tweedledum
