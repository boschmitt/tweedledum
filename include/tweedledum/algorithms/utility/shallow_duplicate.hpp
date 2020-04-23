/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/wire.hpp"

#include <string>

namespace tweedledum {

/*! \defgroup shallow_duplicate This is a hack for documentation :(
 *  \{
 */
/*! \brief Creates a new circuit with same wires as the original.
 *
 * This function requires a template parameter that cannot be inferred.  Useful when duplicating
 * into a different circuit type.
 *
 * \tparam Circuit the original circuit type.
 * \tparam NewCircuit the resulting circuit type.
 * \param[in] original the original quantum circuit (__will not be modified__).
 * \param[in] name the name of the new circuit (default: same as the original).
 * \returns a __new__ circuit without gates.
 */
template<class Circuit, class NewCircuit>
NewCircuit shallow_duplicate(Circuit const& original, std::string_view name = {})
{
	NewCircuit duplicate(name.empty() ? original.name() : name);
	original.foreach_wire([&](wire::id wire, std::string_view name) {
		if (wire.is_qubit()) {
			duplicate.create_qubit(name, original.wire_mode(wire));
		} else {
			duplicate.create_cbit(name);
		}
	});
	return duplicate;
}

/*! \brief Creates a new circuit with same wires as the original.
 *
 * \tparam Circuit the original circuit type.
 * \tparam NewCircuit the resulting circuit type.
 * \param[in] original the original quantum circuit (__will not be modified__).
 * \param[in] name the name of the new circuit (default: same as the original).
 * \returns a __new__ circuit without gates.
 */
template<class Circuit>
Circuit shallow_duplicate(Circuit const& original, std::string_view name = {})
{
	Circuit duplicate(name.empty() ? original.name() : name);
	original.foreach_wire([&](wire::id wire, std::string_view name) {
		if (wire.is_qubit()) {
			duplicate.create_qubit(name, original.wire_mode(wire));
		} else {
			duplicate.create_cbit(name);
		}
	});
	return duplicate;
}
/*! \} */

} // namespace tweedledum
