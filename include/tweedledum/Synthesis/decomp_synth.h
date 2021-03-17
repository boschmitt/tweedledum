/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"

#include <vector>

namespace tweedledum {

/*! \brief Reversible synthesis based on functional decomposition.
 *
 * This is the in-place variant of ``decomp_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 * 
 * \param[inout] circuit A circuit in which the permutation will be synthesized
 * on.
 * \param[in] qubits The wires that will be used.
 * \param[in] cbits The cbits that will be used.
 * \param[in] perm A vector of different integers.
 */
void decomp_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
    std::vector<Cbit> const& cbits, std::vector<uint32_t> const& perm);

/*! \brief Reversible synthesis based on functional decomposition.
 *
 * A permutation is specified as a vector of :math:`2^n` different integers
 * ranging from :math:`0` to :math:`2^n-1`.
 * 
 * \param[in] perm A vector of different integers.
 * \return A reversible circuit.
 */
Circuit decomp_synth(std::vector<uint32_t> const& perm);

} // namespace tweedledum
