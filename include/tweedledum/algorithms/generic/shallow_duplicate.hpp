/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/wire_id.hpp"

#include <string>
#include <type_traits>

namespace tweedledum {

/*! \brief Creates a new network with same wires as the original.
 *
 * This function requires a template parameter that cannot be inferred.  Useful when duplicating 
 * into a different network type.
 * 
 * \param original The original quantum network (will not be modified)
 * \param name The name of the new network (default: same as the original).
 * \return A _new_ network without gates.
 */
template<class NetworkOriginal, class Network>
Network shallow_duplicate(NetworkOriginal const& original, std::string_view name = {})
{
	Network duplicate(name.empty() ? original.name() : name);
	original.foreach_wire([&](wire_id wire, std::string_view name) {
		if (wire.is_qubit()) {
			duplicate.create_qubit(name, original.wire_mode(wire));
		} else {
			duplicate.create_cbit(name);
		}
	});
	return duplicate;
}

/*! \brief Creates a new network with same wires as the original.
 *
 * \param original The original quantum network (will not be modified)
 * \param name The name of the new network (default: same as the original).
 * \return A _new_ network without gates.
 */
template<class Network>
Network shallow_duplicate(Network const& original, std::string_view name = {})
{
	Network duplicate(name.empty() ? original.name() : name);
	original.foreach_wire([&](wire_id wire, std::string_view name) {
		if (wire.is_qubit()) {
			duplicate.create_qubit(name, original.wire_mode(wire));
		} else {
			duplicate.create_cbit(name);
		}
	});
	return duplicate;
}

} // namespace tweedledum
