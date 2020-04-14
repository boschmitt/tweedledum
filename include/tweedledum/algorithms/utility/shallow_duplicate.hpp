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
 * \param original The original quantum circuit (will not be modified).
 * \param name The name of the new circuit (default: same as the original).
 * \return A __new__ circuit without gates.
 */
template<class CircuitOriginal, class Circuit>
Circuit shallow_duplicate(CircuitOriginal const& original, std::string_view name = {})
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

/*! \brief Creates a new circuit with same wires as the original.
 *
 * \param original The original quantum circuit (will not be modified).
 * \param name The name of the new circuit (default: same as the original).
 * \return A __new__ circuit without gates.
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
