/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "shallow_duplicate.hpp"

#include <type_traits>

namespace tweedledum {

/*! \brief Generic function to reverse a network.
 *
 * NOTE: This function requires a template parameter that cannot be inferred.  This is useful when
 * reversing and createing a different network format, e.g. `gg_network` <-> `netlist``
 * 
 * NOTE: Gate type _must_ be the same.
 * 
 * \param original The original quantum network (will not be modified)
 * \return A _new_ reversed network
 */
template<class NewNetwork, class Network>
NewNetwork reverse(Network const& original)
{
	static_assert(std::is_same_v<typename Network::gate_type, typename NewNetwork::gate_type>,
	              "Gate type _must_ be the same");

	NewNetwork result = shallow_duplicate<NewNetwork>(original);
	original.foreach_rgate([&](auto const& node) {
		result.emplace_gate(node.gate);
	});
	return result;
}

/*! \brief Generic function to reverse a network.
 *
 * \param original The original quantum network (will not be modified)
 * \return A _new_ reversed network
 */
template<class Network>
Network reverse(Network const& original)
{
	Network result = shallow_duplicate(original);
	original.foreach_rgate([&](auto const& node) {
		result.emplace_gate(node.gate);
	});
	return result;
}

} // namespace tweedledum
