/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

namespace tweedledum {

/*! \brief Symbolic rotation angles. */
enum class symbolic_angles : uint32_t {
	zero,         // 0, 2π, identity
	one_eighth,   // 2π * (1/8), rotation angle of a T gate
	one_quarter,  // 2π * (1/4), rotation angle of a S gate (phase gate)
	three_eighth, // 2π * (3/8), T gate + S gate
	one_half,     // 2π * (1/2), rotation angle of a Pauli-Z gate
	five_eighth,  // 2π * (1/2), T gate + Pauli-Z gate
	three_fourth, // 2π * (1/2), rotation angle of S† (Conjugate transpose)
	seven_eighth, // 2π * (1/2), rotation angle of T† (Conjugate transpose)
	numerically_defined,
};

/*! \brief Rotation angle datatype.
 *
 * A angle can be defined symbolically or numerically.
 * The numeric value of a rotation angle is given in radians (rad).
 */
class angle {
public:
	constexpr angle(symbolic_angles angle)
	    : symbolic_(angle)
	    , numerical_(0.0)
	{}

	constexpr angle(float angle)
	    : symbolic_(symbolic_angles::numerically_defined)
	    , numerical_(angle)
	{}

	constexpr auto is_symbolic_defined() const
	{
		return symbolic_ != symbolic_angles::numerically_defined;
	}

	constexpr auto symbolic_value() const
	{
		return symbolic_;
	}

	constexpr auto numeric_value() const
	{
		if (symbolic_ == symbolic_angles::numerically_defined) {
			return numerical_;
		}
		auto const factor = static_cast<float>(symbolic_) / 4;
		return static_cast<float>(factor * M_PI);
	}

	bool operator==(symbolic_angles angle) const
	{
		return symbolic_ == angle;
	}

	bool operator==(float angle) const
	{
		return numeric_value() == angle;
	}

	/* When one of the rotation angles is defined numerically, the resulting rotation angle
	 * will be numerically defined.
	 *
	 * The sum of two symbolically defined angles is done using modulo-8 sum.
	 */
	angle& operator+=(angle const& rhs)
	{
		if (!is_symbolic_defined() || !rhs.is_symbolic_defined()) {
			symbolic_ = symbolic_angles::numerically_defined;
			numerical_ += rhs.numeric_value();
			return *this;
		}
		auto angle0 = static_cast<uint32_t>(symbolic_);
		auto angle1 = static_cast<uint32_t>(rhs.symbolic_);
		symbolic_ = static_cast<symbolic_angles>(((angle0 + angle1) % 8));
		return *this;
	}

	friend angle operator+(angle lhs, angle const& rhs)
	{
		if (!lhs.is_symbolic_defined() || !rhs.is_symbolic_defined()) {
			lhs.symbolic_ = symbolic_angles::numerically_defined;
			lhs = lhs.numeric_value() + rhs.numeric_value();
			return lhs;
		}
		auto angle0 = static_cast<uint32_t>(lhs.symbolic_);
		auto angle1 = static_cast<uint32_t>(rhs.symbolic_);
		lhs.symbolic_ = static_cast<symbolic_angles>(((angle0 + angle1) % 8));
		return lhs;
	}

private:
	symbolic_angles symbolic_;
	float numerical_;
};

} // namespace tweedledum
