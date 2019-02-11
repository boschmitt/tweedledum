/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../utils/angle.hpp"
#include "gate_set.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <ostream>

namespace tweedledum {

/*! \brief Simple class to hold information about the operation of a gate */
class gate_base {
public:
#pragma region Constructors
	constexpr gate_base(gate_set operation, angle rotation_angle = 0.0)
	    : operation_(operation)
	    , rotation_angle_(rotation_angle)
	{}

	// gate_base(gate_set operation, angle rotation_angle)
	//     : operation_(operation)
	//     , rotation_angle_(rotation_angle)
	// {
	// 	assert(validate_angle());
	// }
#pragma endregion

#pragma region Operation properties
	/*! \brief Returns the adjoint operation. */
	constexpr auto adjoint() const
	{
		return detail::gates_info[static_cast<uint8_t>(operation_)].adjoint;
	}

	/*! \brief Returns true if this is a meta gate. */
	constexpr auto is_meta() const
	{
		return (operation_ < gate_set::identity || operation_ == gate_set::num_defined_ops);
	}

	/*! \brief Returns true if this gate is a quantum unitary operation. */
	constexpr auto is_unitary_gate() const
	{
		return (operation_ >= gate_set::identity && operation_ <= gate_set::mcz);
	}

	/*! \brief Returns true if this gate acts on a single qubit. */
	constexpr auto is_single_qubit() const
	{
		return (operation_ >= gate_set::input && operation_ <= gate_set::t_dagger);
	}

	/*! \brief Returns true if this gate acts on two qubits. */
	constexpr auto is_double_qubit() const
	{
		return (operation_ == gate_set::cx || operation_ == gate_set::cz);
	}

	/*! \brief Returns true if this gate is a rotation around x axis. */
	constexpr auto is_x_rotation() const
	{
		return detail::gates_info[static_cast<uint8_t>(operation_)].rotation_axis == 'x';
	}

	/*! \brief Returns true if this gate is a rotation around z axis. */
	constexpr auto is_z_rotation() const
	{
		return detail::gates_info[static_cast<uint8_t>(operation_)].rotation_axis == 'z';
	}

	/*! \brief Returns true if this gate is the operation ``operation``. */
	constexpr auto is(gate_set operation) const
	{
		return operation_ == operation;
	}

	template<typename... OPs>
	constexpr auto is_one_of(gate_set operation) const
	{
		return is(operation);
	}

	/*! \brief Returns true if this gate is one of the operations ``{op0, op1, .. , opN}``. */
	template<typename... OPs>
	constexpr auto is_one_of(gate_set op0, OPs... opN) const
	{
		return is(op0) || is_one_of(opN...);
	}

	/*! \brief Returns the operation. (see ``gate_set``) */
	constexpr auto operation() const
	{
		return operation_;
	}
#pragma endregion

#pragma region Angle information
	/* !brief Return the rotation angle */
	constexpr auto rotation_angle() const
	{
		assert(!is_meta());
		return rotation_angle_;
	}
#pragma endregion

#pragma region Overloads
	/* When one of the rotation angles is defined numerically, the resulting rotation angle
	 * will be numerically defined.
	 *
	 * The sum of two symbolically defined angles is done using modulo-8 sum.
	 */
	gate_base& operator+=(gate_base const& rhs)
	{
		assert((is_z_rotation() && rhs.is_z_rotation())
		       || (is_x_rotation() && rhs.is_x_rotation()));

		rotation_angle_ += rhs.rotation_angle_;
		update_operation();
		return *this;
	}

	friend gate_base operator+(gate_base lhs, gate_base const& rhs)
	{
		assert((lhs.is_z_rotation() && rhs.is_z_rotation())
		       || (lhs.is_x_rotation() && rhs.is_x_rotation()));

		lhs.rotation_angle_ += rhs.rotation_angle_;
		lhs.update_operation();
		return lhs;
	}

	friend std::ostream& operator<<(std::ostream& out, gate_base const& operation)
	{
		out << detail::gates_info[static_cast<uint8_t>(operation.operation_)].name;
		return out;
	}
#pragma endregion

private:
	constexpr bool validate_angle() const
	{
		switch (operation_) {
		case gate_set::t:
			return rotation_angle_ == symbolic_angles::one_eighth;

		case gate_set::phase:
			return rotation_angle_ == symbolic_angles::one_quarter;

		case gate_set::pauli_z:
		case gate_set::cz:
		case gate_set::mcz:
		case gate_set::pauli_x:
		case gate_set::cx:
		case gate_set::mcx:
		case gate_set::hadamard:
			return rotation_angle_ == symbolic_angles::one_half;

		case gate_set::phase_dagger:
			return rotation_angle_ == symbolic_angles::three_fourth;

		case gate_set::t_dagger:
			return rotation_angle_ == symbolic_angles::seven_eighth;

		default:
			break;
		}
		return true;
	}

	void update_operation()
	{
		switch (rotation_angle_.symbolic_value()) {
		case symbolic_angles::zero:
			operation_ = gate_set::identity;
			break;

		case symbolic_angles::one_eighth:
			operation_ = gate_set::t;
			break;

		case symbolic_angles::one_quarter:
			operation_ = gate_set::phase;
			break;

		case symbolic_angles::three_eighth:
		case symbolic_angles::five_eighth:
			operation_ = gate_set::rotation_z;
			break;

		case symbolic_angles::one_half:
			operation_ = gate_set::pauli_z;
			break;

		case symbolic_angles::three_fourth:
			operation_ = gate_set::phase_dagger;
			break;

		case symbolic_angles::seven_eighth:
			operation_ = gate_set::t_dagger;
			break;

		default:
			assert(0);
			break;
		}
	}

private:
	gate_set operation_;
	angle rotation_angle_;
};

namespace gate {

/* Single-qubit gates */
constexpr auto identity = gate_base(gate_set::identity, symbolic_angles::zero);
constexpr auto hadamard = gate_base(gate_set::hadamard, symbolic_angles::one_half);
constexpr auto pauli_x = gate_base(gate_set::pauli_x, symbolic_angles::one_half);
constexpr auto t = gate_base(gate_set::t, symbolic_angles::one_eighth);
constexpr auto phase = gate_base(gate_set::phase, symbolic_angles::one_quarter);
constexpr auto pauli_z = gate_base(gate_set::pauli_z, symbolic_angles::one_half);
constexpr auto phase_dagger = gate_base(gate_set::phase_dagger, symbolic_angles::three_fourth);
constexpr auto t_dagger = gate_base(gate_set::t_dagger, symbolic_angles::seven_eighth);

/* Double-qubit unitary gates */
constexpr auto cx = gate_base(gate_set::cx, symbolic_angles::one_half);
constexpr auto cz = gate_base(gate_set::cz, symbolic_angles::one_half);

/* Multiple-qubit unitary gates */
constexpr auto mcx = gate_base(gate_set::mcx, symbolic_angles::one_half);
constexpr auto mcz = gate_base(gate_set::mcz, symbolic_angles::one_half);

} // namespace operation

} // namespace tweedledum