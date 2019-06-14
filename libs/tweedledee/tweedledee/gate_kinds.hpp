/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <string>

namespace tweedledee {

enum class gate_kinds : std::uint8_t {
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

} // namespace tweedledee
