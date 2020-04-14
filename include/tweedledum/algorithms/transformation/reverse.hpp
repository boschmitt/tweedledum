/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../utility/shallow_duplicate.hpp"

#include <type_traits>

namespace tweedledum {

/*! \defgroup reverse This is a hack for documentation :(
 *  \{
 */
/*! \brief Reverse a circuit.
 *
 * __NOTE__: This function requires a template parameter that cannot be inferred.  This is useful
 * when reversing and creating a different circuit representation, e.g. `op_graph` <-> `netlist`
 * 
 * __NOTE__: Operation type __must__ be the same.
 * 
 * \param original The original quantum circuit (__will not be modified__).
 * \return A __new__ reversed circuit.
 */
template<class NewCircuit, class Circuit>
NewCircuit reverse(Circuit const& original)
{
	static_assert(std::is_same_v<typename Circuit::op_type, typename NewCircuit::op_type>,
	              "Operation type _must_ be the same");

	using op_type = typename Circuit::op_type;
	NewCircuit result = shallow_duplicate<NewCircuit>(original);
	original.foreach_rop([&](op_type const& op) {
		result.emplace_op(op);
	});
	return result;
}

/*! \brief Reverse a circuit.
 *
 * __NOTE__: the input and output networs are of the same type.
 * 
 * \param original The original quantum circuit (__will not be modified__).
 * \return A __new__ reversed circuit.
 */
template<class Circuit>
Circuit reverse(Circuit const& original)
{
	return reverse<Circuit, Circuit>(original);
}
/*! \} */

} // namespace tweedledum
