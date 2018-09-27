/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <string>

namespace tweedledum {

/* Maybe I do not need to have different kinds for multple control gates */
enum class gate_kinds_t {
	unknown,
	input,
	output,
	identity,
	// Single-qubit gates
	hadamard,     // Hadamard
	pauli_x,      // Pauli-X gate (aka Not gate)
	pauli_y,      // Pauli-Y gate
	pauli_z,      // Pauli-Z gate
	phase,        // Phase gate (aka S gate or Sqrt(Z))
	phase_dagger, // Conjugate transpose of S gate
	t,            // T gate
	t_dagger,     // Conjugate transpose of T gate
	rotation_x,   // Arbitrary rotation X
	rotation_y,   // Arbitrary rotation Y
	rotation_z,   // Arbitrary rotation Z
	// Two-qubit gates
	cx, // Control Not gate
	cz, // Control Pauli-Z gate
	// Multiple-qubit gates
	mcx, // Multiple Control Not (aka Toffoli) gate
	mcz, // Multiple Control Pauli-Z gate
	mcy, // Multiple Control Pauli-Y gate
};

static const std::string gates_names[] = {
    "Unknown",
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
    "Arbitrary rotation X",
    "Arbitrary rotation Y",
    "Arbitrary rotation Z",
    // Two-qubit gates
    "Control Not",
    "Control Pauli-Z",
    // Multiple-qubit gates
    "Multiple Control Not (aka Toffoli)",
    "Multiple Control Pauli-Z",
    "Multiple Control Pauli-Y",
};

// Determines the name of a gate as used within the front end.
static inline std::string_view gate_name(gate_kinds_t kind)
{
	return gates_names[static_cast<unsigned>(kind)];
}

static inline gate_kinds_t gate_adjoint(gate_kinds_t kind)
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
