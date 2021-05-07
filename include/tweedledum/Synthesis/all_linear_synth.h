/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"
#include "../Utils/LinPhasePoly.h"

#include <vector>

// A CNOT-dihedral circuit is just a fancy way of way the circuit is built using
// only {X, CNOT, Rz} gates.  We know that every circuit written over this gate
// set has a canonical sum-over-paths form.
//
// A sum-over-path form in this case is just a bunch of parities, i.e., linear
// combinations, of the inputs and a respective angle.
//
// To make things clear, take a decomposed Toffoli gate as an example:
//
//                                                             ┌───┐
// x1 ──────────────●───────────────────●─────────●─────────●──┤ R ├
//                  │                   │         │         │  └───┘
//                  │                   │       ┌─┴─┐┌───┐┌─┴─┐┌───┐
// x2 ────●─────────┼─────────●─────────┼───────┤ 5 ├┤ R ├┤ 6 ├┤ R ├
//        │         │         │         │       └───┘└───┘└───┘└───┘
//      ┌─┴─┐┌───┐┌─┴─┐┌───┐┌─┴─┐┌───┐┌─┴─┐                    ┌───┐
// x3 ──┤ 1 ├┤ R ├┤ 2 ├┤ T ├┤ 3 ├┤ R ├┤ 4 ├────────────────────┤ R ├
//      └───┘└───┘└───┘└───┘└───┘└───┘└───┘                    └───┘
//
// The numbered gates are CNOTs.  Their corresponding parities are:
//     (1): x2 + x3
//     (2): x1 + x2 + x3
//     (3): x1 + x3
//     (4): x3
//     (5): x1 + x2
//     (6): x2
// Each of these parities has an associated Rz gate (R in the figure).
//
// The method implemented here generates exactly this.  It will create a circuit
// with all linear combinations and associated Rz gates.
//
// __NOTE__: Keep in mind that the overall linear transformation will be the
// identity.
//
// __NOTE__: This algorithm generate all linear combinations, even when the Rz
// angles are 0.  Thus, this method might not be the best if your sum-over-path
// does not require all parities.
//

namespace tweedledum {

/*! \brief Synthesis of a CNOT-dihedral circuits with all linear combinations.
 *
 * This is the in-place variant of ``all_linear_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 *
 * \param[inout] circuit A circuit in which the parities will be synthesized on.
 * \param[in] qubits The qubits that will be used.
 * \param[in] cbits The cbits that will be used.
 * \param[in] parities List of parities and their associated angles.
 */
void all_linear_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, LinPhasePoly const& parities);

/*! \brief Synthesis of a CNOT-dihedral circuits with all linear combinations.
 *
 * \param[in] num_qubits The number of qubits.
 * \param[in] parities List of parities and their associated angles.
 * \return A CNOT-dihedral circuit on `num_qubits`.
 */
Circuit all_linear_synth(uint32_t num_qubits, LinPhasePoly const& parities);

} // namespace tweedledum
