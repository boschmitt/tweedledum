/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/unitary.hpp"

namespace tweedledum {

/*! \brief Check if two networks are equivalent using unitaries.
 *
 * Needless to say that this method is not scalable at all! But its good to equivalent check small
 * examples and test cases.
 *
 * \tparam Circuit0, Circuit1 the circuit types.
 * \param[in] circuit0, circuit1 the circuits that will be check for equivalence.
 * \param[in] rtol Relative tolerance.
 * \param[in] atol Absolute tolerance.
 * \returns true if the circuits are equivalent.
 */
template<typename Circuit0, typename Circuit1>
bool unitary_verify(Circuit0 const& circuit0, Circuit1 const& circuit1, double const rtol = 1e-05,
                    double const atol = 1e-08)
{
	unitary u0(circuit0);
	unitary u1(circuit1);
	return u0.is_apprx_equal(u1, rtol, atol);
}

} // namespace tweedledum
