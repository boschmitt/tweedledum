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

namespace tweedledum {
namespace linear_synth_detail {

// This function is here because I want to implement the Linear synthe based on
// binary code too
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

} // namespace linear_synth_detail

template<typename Parity>
inline void linear_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    LinearPP<Parity> const& parities)
{
	if (parities.size() == 0) {
		return;
	}
	linear_synth_detail::synthesize(circuit, qubits, parities);
}

template<typename Parity>
inline Circuit linear_synth(
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
	linear_synth(circuit, wires, parities);
	return circuit;
}

} // namespace tweedledum
