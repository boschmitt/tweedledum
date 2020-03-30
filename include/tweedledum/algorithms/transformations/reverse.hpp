/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../generic/shallow_duplicate.hpp"

#include <type_traits>

namespace tweedledum {

/*! \brief Function to reverse a network.
 *
 * NOTE: This function requires a template parameter that cannot be inferred.  This is useful when
 * reversing and createing a different network format, e.g. `op_graph` <-> `netlist``
 * 
 * NOTE: Operation type _must_ be the same.
 * 
 * \param original The original quantum network (will not be modified)
 * \return A _new_ reversed network
 */
template<class NewNetwork, class Network>
NewNetwork reverse(Network const& original)
{
	static_assert(std::is_same_v<typename Network::op_type, typename NewNetwork::op_type>,
	              "Operation type _must_ be the same");

	using op_type = typename Network::op_type;
	NewNetwork result = shallow_duplicate<NewNetwork>(original);
	original.foreach_rop([&](op_type const& op) {
		result.emplace_op(op);
	});
	return result;
}

/*! \brief Function to reverse a network.
 *
 * NOTE: the input and output networs are of the same type.
 * 
 * \param original The original quantum network (will not be modified)
 * \return A _new_ reversed network
 */
template<class Network>
Network reverse(Network const& original)
{
	return reverse<Network, Network>(original);
}

} // namespace tweedledum
