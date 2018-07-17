/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <string>

namespace tweedledum {

enum class gate_kinds : std::uint8_t {
	input,
	output,
	identity,
	// Single-qubit Clifford gates
	pauli_x,      // Pauli-X gate
	pauli_y,      // Pauli-Y gate
	pauli_z,      // Pauli-Z gate
	phase,        // Phase aka Sqrt(Z) gate
	phase_dagger, // Conjugate transpose of S
	hadamard,     // Hadamard
	// Single-qubit non-Clifford gates
	t,        // T-gate
	t_dagger, // Conjugate transpose of T-gate
	// Multiple-qubit gates
	cnot, // Controlled not gate
	unknown,
};

static const std::string token_names[] = {
    "Input",
    "Output",
    "Identity",
    "Pauli-X",
    "Pauli-Y",
    "Pauli-Z",
    "Phase (aka Sqrt(Z))",
    "Conjugate transpose of Phase",
    "Hadamard",
    "T",
    "Conjugate transpose of T",
    "Controlled not",
    "Unknown",
};

// Determines the name of a gate as used within the front end.
inline std::string_view gate_name(gate_kinds kind)
{
	return token_names[static_cast<std::uint8_t>(kind)];
}

inline gate_kinds gate_adjoint(gate_kinds kind)
{ 
	switch (kind) {
	case gate_kinds::hadamard:
		return gate_kinds::hadamard;

	case gate_kinds::phase:
		return gate_kinds::phase_dagger;

	case gate_kinds::phase_dagger:
		return gate_kinds::phase;

	case gate_kinds::t:
		return gate_kinds::t_dagger;

	case gate_kinds::t_dagger:
		return gate_kinds::t;

	default:
		return gate_kinds::unknown;
	}
}

} // namespace tweedledum
