/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../Utils/Matrix.h"

#include <nlohmann/json.hpp>
#include <vector>

// This implementation is based on:
//
// Patel, Ketan N., Igor L. Markov, and John P. Hayes. "Optimal synthesis of
// linear reversible circuits."  Quantum Information & Computation 8.3 (2008):
// 282-294.
//
// Linear reversible classical circuits form an important subclass of quantum
// circuits, which can be generated by using only the CX (CNOT) gate.
//
// Other quantum circuit synthesis algorithms, e.g., Gray-Synth, require the
// synthesis of linear reversible circuits or can generate circuits with blocks
// of CX gates.  Therefore, synthesis methods that reduce the size of these
// circuits or sub-blocks would, in turn, reduce the size of the overall
// quantum circuit as well.
//
// What is the problem we are trying to solve here?
//
//    Synthesize an arbitrary linear reversible circuit on N qubits using as
//    few CX gates as possible.
//
// This problem can be mapped to the problem of row reducing an N × N binary
// matrix.  Patel et al. presented an algorithm that is asymptotically optimal
// up to a multiplicative constant. Their algorithm can be understood as a
// matrix decomposition algorithm that yields an asymptotically efficient
// elementary matrix decomposition of a binary matrix.
//
namespace tweedledum {

/*! \brief Synthesis of linear reversible circuits (CNOT synthesis).
 *
 * This is the in-place variant of ``linear_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 *
 * \param[inout] circuit A circuit in which the linear transformation will be
 * synthesized on.
 * \param[in] qubits The qubits that will be used.
 * \param[in] cbits The cbits that will be used.
 * \param[in] matrix An N x N binary matrix.
 */
void linear_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, BMatrix const& matrix,
  nlohmann::json const& config = {});

/*! \brief Synthesis of linear reversible circuits (CNOT synthesis).
 *
 * \param[in] matrix An N x N binary matrix.
 * \return A linear reversible circuit on N qubits.
 */
Circuit linear_synth(BMatrix const& matrix, nlohmann::json const& config = {});

} // namespace tweedledum