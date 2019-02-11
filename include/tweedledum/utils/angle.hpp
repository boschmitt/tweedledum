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
	/*! \brief 0, 2π, identity */
	zero,
	/*! \brief 2π * (1/8), rotation angle of a T gate */
	one_eighth,
	/*! \brief 2π * (1/4), rotation angle of a S gate (phase gate) */
	one_quarter,
	/*! \brief 2π * (3/8), T gate + S gate */
	three_eighth,
	/*! \brief 2π * (1/2), rotation angle of a Pauli-Z gate, Pauli-X (NOT) */
	one_half,
	/*! \brief 2π * (1/2), T gate + Pauli-Z gate */
	five_eighth,
	/*! \brief 2π * (1/2), rotation angle of S† (Conjugate transpose) */
	three_fourth,
	/*! \brief 2π * (1/2), rotation angle of T† (Conjugate transpose) */
	seven_eighth,
	/*! \brief Indicates that the angle is numercally defined */
	numerically_defined,
};

/*! \brief Simple class to represent rotation angles
 *
 * A angle can be defined symbolically or numerically.
 * The numeric value of a rotation angle is given in radians (rad).
 */
class angle {
public:
#pragma region Constructors
	constexpr angle(symbolic_angles angle)
	    : symbolic_(angle)
	    , numerical_(0.0)
	{}

	constexpr angle(double angle)
	    : symbolic_(symbolic_angles::numerically_defined)
	    , numerical_(angle)
	{}
#pragma endregion

#pragma region Properties
	/*! \brief Returns true if this angle is symbolically defined. */
	constexpr auto is_symbolic_defined() const
	{
		return symbolic_ != symbolic_angles::numerically_defined;
	}

	/*! \brief Returns the symbolic value of this angle. */
	constexpr auto symbolic_value() const
	{
		return symbolic_;
	}

	/*! \brief Returns the numeric value of this angle. */
	constexpr auto numeric_value() const
	{
		if (symbolic_ == symbolic_angles::numerically_defined) {
			return numerical_;
		}
		auto const factor = static_cast<double>(symbolic_) / 4;
		return static_cast<double>(factor * M_PI);
	}
#pragma endregion

#pragma region Overloads
	bool operator==(symbolic_angles angle) const
	{
		return symbolic_ == angle;
	}

	bool operator==(double angle) const
	{
		return numeric_value() == angle;
	}

	bool operator!=(symbolic_angles angle) const
	{
		return symbolic_ != angle;
	}

	bool operator!=(double angle) const
	{
		return numeric_value() != angle;
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
#pragma endregion

private:
	symbolic_angles symbolic_;
	double numerical_;
};

} // namespace tweedledum
