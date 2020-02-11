/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "shallow_duplicate.hpp"

#include <type_traits>

namespace tweedledum {

/*! \brief Generic rewrite function.
 *
 * The parameter ``fn`` is any callable that must have the following signature:
 * - ``bool(network_type&, gate_type const&)``
 * 
 * If ``fn`` returns ``true`` then it means that the gate was rewritten and nothing else will be
 * done.  In case it returns ``false``, then it means the current gate must be copied to the
 * new network.
 * 
 * NOTE: This function requires a template parameter that cannot be inferred.  This is useful when
 * rewriting into a different network format, e.g. `gg_network` <-> `netlist``
 * 
 * NOTE: Gate type _must_ be the same.
 * 
 * \param original The original quantum network (will not be modified)
 * \param fn A rewriting function with the signature: ``bool(network_type&, gate_type const&)``
 * \param num_ancillae Number of ancillae to add to the network (default: 0)
 * \return A _new_ rewritten network.
 */
template<class NewNetwork, class Network, class RewriteFn>
NewNetwork rewrite_network(Network const& original, RewriteFn&& fn, uint32_t num_ancillae = 0u)
{
	static_assert(std::is_same_v<typename Network::gate_type, typename NewNetwork::gate_type>,
	              "Gate type _must_ be the same");
	static_assert(std::is_invocable_r_v<bool, RewriteFn, Network&, typename Network::gate_type>,
	              "The rewriting function signature _must_ be ``bool(network, gate)``");

	    NewNetwork result
	    = shallow_duplicate<NewNetwork>(original);
	for (uint32_t i = 0u; i < num_ancillae; ++i) {
		result.add_qubit(/* is_ancilla */ true);
	}
	original.foreach_gate([&](auto const& node) {
		if (!fn(result, node.gate)) {
			result.emplace_gate(node.gate);
		}
	});
	result.rewire(original.wiring_map());
	return result;
}

/*! \brief Generic network rewriting function
 *
 * The parameter ``fn`` is any callable that must have the following signature:
 * - ``bool(network_type&, gate_type const&)``
 * 
 * If ``fn`` returns ``true`` then it means that the gate was rewritten and nothing else will be
 * done.  In case it returns ``false``, then it means the current gate must be copied to the
 * new network.
 * 
 * \param original The original quantum network (will not be modified)
 * \param fn A rewriting function with the signature: ``bool(network_type&, gate_type const&)``
 * \param num_ancillae Number of ancillae to add to the network (default: 0)
 * \return A _new_ rewritten network.
 */
template<class Network, class RewriteFn>
Network rewrite_network(Network const& original, RewriteFn&& fn, uint32_t num_ancillae = 0u)
{
	static_assert(std::is_invocable_r_v<bool, RewriteFn, Network&, typename Network::gate_type const&>,
	              "The rewriting function signature _must_ be ``bool(network, gate)``");

	Network result = shallow_duplicate(original);
	for (auto i = 0u; i < num_ancillae; ++i) {
		result.add_qubit(/* is_ancilla */ true);
	}
	original.foreach_gate([&](auto const& node) {
		if (!fn(result, node.gate)) {
			result.emplace_gate(node.gate);
		}
	});
	result.rewire(original.wiring_map());
	return result;
}

} // namespace tweedledum
