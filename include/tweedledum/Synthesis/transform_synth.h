/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"

#include <vector>

// This implementation is based on:
//
// Miller, D. Michael, Dmitri Maslov, and Gerhard W. Dueck. "A transformation
// based algorithm for reversible logic synthesis." Proceedings 2003. design
// automation conference (ieee cat. no. 03ch37451). IEEE, 2003.
//
// Starting from a reversible function, transformation-based synthesis applies
// gates and adjusts the function representation accordingly in a way that each
// gate application gets the function closer to the identity function.  If the
// identity function has been reached, all applied gates make up for the circuit
// that realizes the initial function.
//
// Here there is also the implementation of a multidirectional method based on:
//
// Soeken, Mathias, Gerhard W. Dueck, and D. Michael Miller. "A fast symbolic
// transformation based algorithm for reversible logic synthesis." International
// Conference on Reversible Computation. Springer, Cham, 2016.
//
// Variants:
// (*) unidirectional: only adds gates from the output side
// (*) bidirectional: adds gates from input __or__ output side at each step
// (*) multidirectional: adds gates from input __and__ output side at each step
//
namespace tweedledum {

/*! \brief Reversible synthesis based on symbolic transformation.
 *
 * This is the in-place variant of ``transform_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 *
 * \param[inout] circuit A circuit in which the permutation will be synthesized
 * on.
 * \param[in] qubits The qubits that will be used.
 * \param[in] cbits The cbits that will be used.
 * \param[in] perm A vector of different integers.
 */
void transform_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, std::vector<uint32_t> const& perm);

/*! \brief Reversible synthesis based on symbolic transformation.
 *
 * A permutation is specified as a vector of :math:`2^n` different integers
 * ranging from :math:`0` to :math:`2^n-1`.
 *
 * \param[in] perm A vector of different integers.
 * \return A reversible circuit.
 */
Circuit transform_synth(std::vector<uint32_t> const& perm);

} // namespace tweedledum
