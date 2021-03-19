/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"
#include "../Utils/LinPhasePoly.h"
#include "../Utils/Matrix.h"

#include <nlohmann/json.hpp>
#include <vector>

// This implementation is based on:
//
// Amy, Matthew, Parsiad Azimzadeh, and Michele Mosca. "On the controlled-NOT
// complexity of controlled-NOT–phase circuits." Quantum Science and Technology
// 4.1 (2018): 015002.
//
// This synthesis method generates a CNOT-dihedral circuit.  In principle it
// serves the same purpose of 'all_linear_synth' algorithm, but with two
// important differences:
//     (1) it will __not__ necessarily generate all possible linear
//         combinations.  Thus, the synthesized circuit can potentially be of
//         better.
//     (2) The overall linear transformation can be defined quality
//         (in the number of gates).
//
// __NOTE__: if you indeed require all linear combinations, then
//           all_linear_synth __will be faster__.
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
 * \param[in] linear_trans The overall linear transformation
 * \param[in] parities List of parities and their associated angles.
 */
// Each column is a parity, num_rows = num_qubits
void gray_synth(Circuit& circuit, std::vector<Qubit> const& qubits, 
    std::vector<Cbit> const& cbits, BMatrix linear_trans, LinPhasePoly parities,
    nlohmann::json const& config = {});

/*! \brief Synthesis of a CNOT-dihedral circuits.
 *
 * \param[in] num_qubits The number of qubits.
 * \param[in] parities List of parities and their associated angles.
 * \return A CNOT-dihedral circuit on `num_qubits`.
 */
Circuit gray_synth(uint32_t num_qubits, LinPhasePoly const& parities,
    nlohmann::json const& config = {});

} // namespace tweedledum
