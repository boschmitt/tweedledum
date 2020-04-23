/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/node.hpp"
#include "../../views/layers_view.hpp"
#include "../utility/shallow_duplicate.hpp"

#include <vector>

namespace tweedledum {

/*! \brief As soon as possible (ASAP) rescheduler
 *
 * In tweedledum, the DAG circuit representations are always topologically sorted.  The topological
 * order, however, is not guaranteed to be layered. Meaning that when topologically visiting
 * operations, you might visit a node of the second layer before visiting all nodes of the first
 * layer.  For example:
 \verbatim
                              ┌───┐
                  >───●───────┤ 4 ├    visiting order: [1] [2] [3] [4] [5]
                      │       └───┘    layer:           1   2   3   2   1
                    ┌─┴─┐┌───┐┌───┐
                  >─┤ 1 ├┤ 2 ├┤ 3 ├
                    └───┘└───┘└───┘
                              ┌───┐
                  >───────────┤ 5 ├
                              └───┘
 \endverbatim
 * The nodes are numbered as they appear in the underlying DAG data structure.  Nodes will be
 * visited in this order when using ``foreach_node`` or ``foreach_op`` methods.  Observer that node
 * five is visited last, but it is on the first layer.
 *
 * This algorithm will move operations closer to inputs, hence guaranteeing that all nodes of the
 * same layer are visited before visiting the nodes of the next layer. Applying to the example we
 * obtain:
 \verbatim
                         ┌───┐
                  >───●──┤ 4 ├─────    visiting order: [1] [5] [2] [4] [3]
                      │  └───┘         layer:           1   1   2   2   3
                    ┌─┴─┐┌───┐┌───┐
                  >─┤ 1 ├┤ 2 ├┤ 3 ├
                    └───┘└───┘└───┘
                    ┌───┐
                  >─┤ 5 ├──────────
                    └───┘
 \endverbatim
 *
 * __NOTE__: The ``node_id``s are not preserved.
 *
 * \tparam Circuit the circuit type.
 * \param[in] original the original quantum circuit (__will not be modified__).
 * \returns a __new__ rescheduled (leyered) circuit.
 */
template<typename Circuit>
Circuit asap_reschedule(Circuit const& original)
{
	Circuit rescheduled = shallow_duplicate(original);
	layers_view layers(original);
	// Start from the first layer of gates, i.e., 1u
	for (uint32_t layer = 1u; layer < layers.num_layers(); ++layer) {
		std::vector<node::id> const nodes = layers.layer(layer);
		for (node::id const n_id : nodes) {
			rescheduled.emplace_op(layers.node(n_id).op);
		}
	}
	return rescheduled;
}

} // namespace tweedledum
