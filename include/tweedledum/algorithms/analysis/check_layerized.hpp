/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../views/layers_view.hpp"

#include <vector>

namespace tweedledum {

/*! \brief Check is a network is layerized
 *
 */
template<typename Network>
bool check_layerized(Network const& network)
{
	using node_type = typename Network::node_type;
	layers_view layered_ntk(network);
	uint32_t layer = 0u;
	bool is_layerized = true;
	layered_ntk.foreach_node([&](node_type const& node) {
		if (layered_ntk.layer(node) < layer) {
			is_layerized = false;
		}
		layer = std::max(layer, layered_ntk.layer(node));
	});
	return is_layerized;
}

} // namespace tweedledum
