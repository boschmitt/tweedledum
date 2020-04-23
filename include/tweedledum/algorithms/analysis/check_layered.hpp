/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../views/layers_view.hpp"

namespace tweedledum {

/*! \brief Check if a DAG circuit representation is layered.
 *
 * A topological order is defined to be layered if all nodes of the i-th layer appear before all 
 * nodes of the (i+1)-th layer.
 * 
 * \tparam Circuit the type of the circuit, must be a DAG.
 * \param[in] circuit the DAG circuit.
 * \returns true if the circuit is layered.
 */
template<typename Circuit>
bool check_layered(Circuit const& circuit)
{
	using node_type = typename Circuit::node_type;
	layers_view layered(circuit);
	uint32_t layer = 0u;
	bool is_layered = true;
	layered.foreach_node([&](node_type const& node) {
		if (layered.layer(node) < layer) {
			is_layered = false;
		}
		layer = std::max(layer, layered.layer(node));
	});
	return is_layered;
}

} // namespace tweedledum
