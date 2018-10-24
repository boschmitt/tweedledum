/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <string>
#include <string_view>

namespace tweedledum {

/* Maybe I do not need to have different kinds for multple control gates */
enum class gate_kinds_t : uint32_t {
	/* Meta gates */
	unknown,
	input,
	output,

	/* Single-qubit gates */
	// Powers of T
	identity,     // T^0
	t,            // T^1, T gate
	phase,        // T^2, aka S gate or Sqrt(Z)
	t3,           // T^3, T gate + S gate
	pauli_z,      // T^4
	t5,           // T^5, T gate + Pauli-Z gate
	phase_dagger, // T^6, Conjugate transpose of S gate
	t_dagger,     // T^7, Conjugate transpose of T gate

	hadamard,
	pauli_x,    // aka Not gate
	pauli_y,
	rotation_x, // Arbitrary rotation X
	rotation_y, // Arbitrary rotation Y
	rotation_z, // Arbitrary rotation Z

	/* Two-qubit gates */
	cx, // Control Not gate
	cz, // Control Pauli-Z gate

	/* Multiple-qubit gates */
	mcx, // Multiple Control Not (aka Toffoli) gate
	mcz, // Multiple Control Pauli-Z gate
	mcy, // Multiple Control Pauli-Y gate

	num_gate_kinds,
};

static const std::string gates_names[] = {
    /* Meta gates */
    "Unknown",
    "Input",
    "Output",
    /* Single-qubit gates */
    "Identity",
    "T",
    "Phase aka Sqrt(Z)",
    "T^3",
    "Pauli-Z",
    "T^5",
    "Conjugate transpose of Phase",
    "Conjugate transpose of T",
    "Hadamard",
    "Pauli-X",
    "Pauli-Y",
    "Arbitrary rotation X",
    "Arbitrary rotation Y",
    "Arbitrary rotation Z",
    /* Two-qubit gates */
    "Control Not",
    "Control Pauli-Z",
    /* Multiple-qubit gates */
    "Multiple Control Not (aka Toffoli)",
    "Multiple Control Pauli-Z",
    "Multiple Control Pauli-Y",
    "ERROR",
};

// Determines the name of a gate as used within the front end.
static inline std::string_view gate_name(gate_kinds_t kind)
{
	return gates_names[static_cast<uint32_t>(kind)];
}

static inline gate_kinds_t gate_adjoint(gate_kinds_t kind)
{
	switch (kind) {
	case gate_kinds_t::hadamard:
		return gate_kinds_t::hadamard;

	case gate_kinds_t::pauli_x:
		return gate_kinds_t::pauli_x;

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

static inline gate_kinds_t gate_merge_z_rotations(gate_kinds_t kind0, gate_kinds_t kind1)
{
	if (kind0 == gate_kinds_t::rotation_z || kind1 == gate_kinds_t::rotation_z) {
		return gate_kinds_t::rotation_z;
	}
	constexpr auto id = static_cast<uint32_t>(gate_kinds_t::identity);
	auto k0 = static_cast<uint32_t>(kind0) - id;
	auto k1 = static_cast<uint32_t>(kind1) - id;
	return static_cast<gate_kinds_t>(((k0 + k1) % 8) + id);
}

static inline bool gate_is_z_rotation(gate_kinds_t kind)
{
	switch (kind) {
	case gate_kinds_t::t:
	case gate_kinds_t::phase:
	case gate_kinds_t::t3:
	case gate_kinds_t::pauli_z:
	case gate_kinds_t::t5:
	case gate_kinds_t::phase_dagger:
	case gate_kinds_t::t_dagger:
	case gate_kinds_t::rotation_z:
	case gate_kinds_t::cz:
	case gate_kinds_t::mcz:
		return true;

	default:
		break;
	}
	return false;
}

} // namespace tweedledum
