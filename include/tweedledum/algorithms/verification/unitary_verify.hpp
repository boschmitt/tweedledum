/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/unitary.hpp"

namespace tweedledum {

/* \brief Check if two networks are equivalent using unitaries.
 *
 * Needless to say that this method is not scalable at all! But its good to equivalent check small
 * examples and test cases.
 * 
 * \params rtol Relative tolerance (default: 1e-05).
 * \params atol Absolute tolerance (default: 1e-08).
 */
template<typename Network0, typename Network1>
bool unitary_verify(Network0 const& network0, Network1 const& network1, double const rtol = 1e-05,
	                    double const atol = 1e-08)
{
	unitary u0(network0);
	unitary u1(network1);
	return u0.is_apprx_equal(u1, rtol, atol);
}

} // namespace tweedledum
