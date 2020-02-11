/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/io_id.hpp"

#include <string>
#include <type_traits>

namespace tweedledum {

/*! \brief Creates a new network with same I/O's as the original.
 *
 * This function requires a template parameter that cannot be inferred.  Useful when duplicate into
 * a different network type.
 * 
 * \param original The original quantum network (will not be modified)
 * \param name The name of the new network (default: same as the original).
 * \return A _new_ network without gates.
 */
template<class NetworkOriginal, class Network>
Network shallow_duplicate(NetworkOriginal const& original, std::string_view name = {})
{
	Network duplicate(name.empty() ? original.name() : name);
	original.foreach_io([&](io_id io, std::string const& label) {
		if (io.is_qubit()) {
			io_id new_io = duplicate.add_qubit(label);
			if (original.is_ancilla(io)) {
				duplicate.mark_as_ancilla(new_io);
			}
			if (original.is_input(io)) {
				duplicate.mark_as_input(new_io);
			}
			if (original.is_output(io)) {
				duplicate.mark_as_output(new_io);
			}
		} else {
			duplicate.add_cbit(label);
		}
	});
	return duplicate;
}

/*! \brief Creates a new network with same I/O's as the original.
 *
 * \param original The original quantum network (will not be modified)
 * \param name The name of the new network (default: same as the original).
 * \return A _new_ network without gates.
 */
template<class Network>
Network shallow_duplicate(Network const& original, std::string_view name = {})
{
	Network duplicate(name.empty() ? original.name() : name);
	original.foreach_io([&](io_id io, std::string const& label) {
		if (io.is_qubit()) {
			io_id new_io = duplicate.add_qubit(label);
			if (original.is_ancilla(io)) {
				duplicate.mark_as_ancilla(new_io);
			}
			if (original.is_input(io)) {
				duplicate.mark_as_input(new_io);
			}
			if (original.is_output(io)) {
				duplicate.mark_as_output(new_io);
			}
		} else {
			duplicate.add_cbit(label);
		}
	});
	return duplicate;
}

} // namespace tweedledum
