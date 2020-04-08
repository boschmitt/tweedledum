/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../views/layers_view.hpp"
#include "../generic/shallow_duplicate.hpp"

#include <vector>

namespace tweedledum {

/*! \brief As soon as possible (ASAP) rescheduler
 *
 * Move operations closer to inputs
 */
template<typename Network>
Network asap_reschedule(Network const& original)
{
	Network rescheduled = shallow_duplicate(original);

	layers_view layered_original(original);
	// Start from the first layer of gates, i.e., 1u
	for (uint32_t layer = 1u; layer < layered_original.num_layers(); ++layer) {
		std::vector<node_id> const nodes = layered_original.layer(layer);
		for (node_id const nid : nodes) {
			rescheduled.emplace_op(layered_original.node(nid).op);
		}
	}
	return rescheduled;
}

} // namespace tweedledum
