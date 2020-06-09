/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/LinearPP.h"
#include "../../support/Matrix.h"

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
#pragma region Implementation details
namespace all_linear_synth_detail {

// I added this level of indirection because I can implement this method with
// other codes or just using binary sequence.
template<typename Parity>
void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits,
    LinearPP<Parity> parities)
{
	// Generate Gray code
	std::vector<uint32_t> gray_code(1u << circuit.num_qubits());
	for (uint32_t i = 0; i < (1u << circuit.num_qubits()); ++i) {
		gray_code.at(i) = ((i >> 1) ^ i);
	}

	// Initialize the parity of each qubit state
	// Applying phase gate to parities that consisting of just one variable
	// i is the index of the target
	std::vector<uint32_t> qubits_states(circuit.num_qubits(), 0);
	for (uint32_t i = 0u; i < circuit.num_qubits(); ++i) {
		qubits_states[i] = (1u << i);
		auto angle = parities.extract_term(qubits_states[i]);
		if (angle != 0.0) {
			circuit.create_instruction(
			    GateLib::R1(angle), {qubits[i]});
		}
	}

	for (uint32_t i = circuit.num_qubits() - 1; i > 0; --i) {
		for (uint32_t j = (1u << (i + 1)) - 1; j > (1u << i); --j) {
			uint32_t c0
			    = std::log2(gray_code[j] ^ gray_code[j - 1u]);
			circuit.create_instruction(
			    GateLib::X(), {qubits[c0]}, qubits[i]);

			qubits_states[i] ^= qubits_states[c0];
			auto angle = parities.extract_term(qubits_states[i]);
			if (angle != 0.0) {
				circuit.create_instruction(
				    GateLib::R1(angle), {qubits[i]});
			}
		}
		uint32_t c1 = std::log2(
		    gray_code[1 << i] ^ gray_code[(1u << (i + 1)) - 1u]);
		circuit.create_instruction(
		    GateLib::X(), {qubits[c1]}, qubits[i]);

		qubits_states[i] ^= qubits_states[c1];
		auto angle = parities.extract_term(qubits_states[i]);
		if (angle != 0.0) {
			circuit.create_instruction(
			    GateLib::R1(angle), {qubits[i]});
		}
	}
}

} // namespace all_linear_synth_detail
#pragma endregion

/*! \brief Synthesis of a CNOT-dihedral circuits with all linear combinations.
 *
 * This is the in-place variant of ``all_linear_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 * 
 * \param[inout] circuit A circuit in which the parities will be synthesized on.
 * \param[in] qubits The qubits that will be used.
 * \param[in] parities List of parities and their associated angles.
 */
template<typename Parity>
inline void all_linear_synth(Circuit& circuit,
    std::vector<WireRef> const& qubits, LinearPP<Parity> const& parities)
{
	if (parities.size() == 0) {
		return;
	}
	all_linear_synth_detail::synthesize(circuit, qubits, parities);
}

/*! \brief Synthesis of a CNOT-dihedral circuits with all linear combinations.
 *
 * \param[in] num_qubits The number of qubits.
 * \param[in] parities List of parities and their associated angles.
 * \return A CNOT-dihedral circuit on `num_qubits`.
 */
template<typename Parity>
inline Circuit all_linear_synth(
    uint32_t num_qubits, LinearPP<Parity> const& parities)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");

	// Create the necessary qubits
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	all_linear_synth(circuit, wires, parities);
	return circuit;
}

} // namespace tweedledum
