/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include <string>

namespace tweedledum {

enum class gate_kinds_t {
	input,
	output,
	identity,
	// Single-qubit gates
	hadamard,     // Hadamard
	pauli_x,      // Pauli-X gate (aka Not gate)
	pauli_y,      // Pauli-Y gate
	pauli_z,      // Pauli-Z gate
	phase,        // Phase aka Sqrt(Z) gate
	phase_dagger, // Conjugate transpose of S
	t,            // T gate
	t_dagger,     // Conjugate transpose of T gate
	rotation_x,   // Rotation X
	rotation_y,   // Rotation Y
	rotation_z,   // Rotation Z
	// Two-qubit gates
	cx, // Control Not gate
	cz, // Control Pauli-Z gate
	// Multiple-qubit gates
	mcx, // Multiple Control Not (aka Toffoli) gate
	mcz, // Multiple Control Pauli-Z gate
	mcy, // Multiple Control Pauli-Y gate
	unknown,
};

static const std::string token_names[] = {
    "Input",
    "Output",
    "Identity",
    // Single-qubit gates
    "Hadamard",
    "Pauli-X",
    "Pauli-Y",
    "Pauli-Z",
    "Phase aka Sqrt(Z)",
    "Conjugate transpose of Phase",
    "T",
    "Conjugate transpose of T",
    "Rotation X",
    "Rotation Y",
    "Rotation Z",
    // Two-qubit gates
    "Control Not",
    "Control Pauli-Z",
    // Multiple-qubit gates
    "Multiple Control Not (aka Toffoli)",
    "Multiple Control Pauli-Z",
    "Multiple Control Pauli-Y",
    "Unknown",
};

// Determines the name of a gate as used within the front end.
inline std::string_view gate_name(gate_kinds_t kind)
{
	return token_names[static_cast<unsigned>(kind)];
}

inline gate_kinds_t gate_adjoint(gate_kinds_t kind)
{
	switch (kind) {
	case gate_kinds_t::hadamard:
		return gate_kinds_t::hadamard;

	case gate_kinds_t::phase:
		return gate_kinds_t::phase_dagger;

	case gate_kinds_t::phase_dagger:
		return gate_kinds_t::phase;

	case gate_kinds_t::t:
		return gate_kinds_t::t_dagger;

	case gate_kinds_t::t_dagger:
		return gate_kinds_t::t;

	default:
		return gate_kinds_t::unknown;
	}
}

} // namespace tweedledum
