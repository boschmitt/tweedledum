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
 * \tparam Circuit the original circuit type.
 * \tparam NewCircuit the resulting circuit type.
 * \param[in] original the original quantum circuit (__will not be modified__).
 * \param[in] fn a rewriting function with the signature: ``bool(network_type&, gate_type const&)``.
 * \param[in] num_ancillae number of ancillae to add to the network.
 * \returns a _new_ rewritten network.
 */
template<class NewCircuit, class Circuit, class RewriteFn>
NewCircuit rewrite_network(Circuit const& original, RewriteFn&& fn, uint32_t num_ancillae = 0u)
{
	static_assert(std::is_same_v<typename Circuit::gate_type, typename NewCircuit::gate_type>,
	              "Gate type _must_ be the same");
	static_assert(std::is_invocable_r_v<bool, RewriteFn, Circuit&, typename Circuit::gate_type>,
	              "The rewriting function signature _must_ be ``bool(network, gate)``");

	NewCircuit result = shallow_duplicate<NewCircuit>(original);
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
 * \tparam Circuit the original circuit type.
 * \tparam NewCircuit the resulting circuit type.
 * \param[in] original the original quantum circuit (__will not be modified__).
 * \param[in] fn a rewriting function with the signature: ``bool(network_type&, gate_type const&)``.
 * \param[in] num_ancillae number of ancillae to add to the network.
 * \returns a _new_ rewritten network.
 */
template<class Circuit, class RewriteFn>
Circuit rewrite_network(Circuit const& original, RewriteFn&& fn, uint32_t num_ancillae = 0u)
{
	static_assert(
	    std::is_invocable_r_v<bool, RewriteFn, Circuit&, typename Circuit::gate_type const&>,
	    "The rewriting function signature _must_ be ``bool(network, gate)``");

	Circuit result = shallow_duplicate(original);
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
