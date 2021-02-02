/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cmath>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <numeric>
#include <optional>
#include <utility>

namespace tweedledum {

constexpr double PI_k = 3.14159265358979323846;
constexpr double PI_2_k = 1.57079632679489661923;
constexpr double PI_4_k = 0.78539816339744830962;
constexpr long double PI_lk = 3.141592653589793238462643383279502884L;

/*! \brief Simple class to represent rotation angles
 *
 * A angle can be defined symbolically or numerically. When defined symbolic the angle is always a
 * multiple of pi, i.e., the symbolic value will always multiplied by pi.
 *
 * The numeric value of a rotation angle is given in radians (rad).
 */
class Angle {
    using fraction_type = std::pair<int32_t, int32_t>;

public:
#pragma region Constructors
    constexpr Angle(int32_t numerator, int32_t denominator)
        : numerator_(numerator)
        , denominator_(denominator)
        , numerical_(0)
    {
        assert(denominator != 0 && "Denominator cannot be 0");
        normalize();
    }

    constexpr Angle(double angle)
        : numerator_(0)
        , denominator_(0)
        , numerical_(angle)
    {}
#pragma endregion

#pragma region Properties
    /*! \brief Returns true if this angle is symbolically defined. */
    constexpr bool is_numerically_defined() const
    {
        return denominator_ == 0;
    }

    /*! \brief Returns the symbolic value of this angle. */
    constexpr std::optional<fraction_type> symbolic_value() const
    {
        if (is_numerically_defined()) {
            return std::nullopt;
        }
        return std::make_pair(numerator_, denominator_);
    }

    /*! \brief Returns the numeric value of this angle. */
    constexpr double numeric_value() const
    {
        return numerical_;
    }
#pragma endregion

#pragma region Overloads
    constexpr Angle operator-() const
    {
        Angle result = *this;
        if (result.numerical_ == 0.0) {
            return result;
        }
        if (!is_numerically_defined()) {
            result.numerator_ = -numerator_;
        }
        result.numerical_ = -numerical_;
        return result;
    }

    bool operator==(Angle const& other) const
    {
        return numeric_value() == other.numeric_value();
    }

    bool operator!=(Angle const& other) const
    {
        return numeric_value() != other.numeric_value();
    }

    /* When one of the rotation angles is defined numerically, the resulting rotation angle
     * will be numerically defined.
     */
    Angle& operator+=(Angle const& rhs)
    {
        if (is_numerically_defined() || rhs.is_numerically_defined()) {
            denominator_ = 0;
            numerical_ += rhs.numeric_value();
            return *this;
        }
        numerator_ = (numerator_ * rhs.denominator_) + (rhs.numerator_ * denominator_);
        denominator_ = denominator_ * rhs.denominator_;
        normalize();
        return *this;
    }

    friend Angle operator+(Angle lhs, Angle const& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Angle operator-(Angle lhs, Angle const& rhs)
    {
        lhs += -rhs;
        return lhs;
    }

    friend Angle operator*(Angle lhs, int32_t const& rhs)
    {
        if (lhs.is_numerically_defined()) {
            lhs.denominator_ = 0;
            lhs.numerical_ = lhs.numeric_value() * rhs;
            return lhs;
        }
        lhs.numerator_ = (lhs.numerator_ * rhs);
        lhs.normalize();
        return lhs;
    }

    friend Angle operator/(Angle lhs, int32_t const& rhs)
    {
        if (lhs.is_numerically_defined()) {
            lhs.denominator_ = 0;
            lhs.numerical_ = lhs.numeric_value() / rhs;
            return lhs;
        }
        lhs.denominator_ = (lhs.denominator_ * rhs);
        lhs.normalize();
        return lhs;
    }

    friend std::ostream& operator<<(std::ostream& os, Angle const& angle)
    {
        if (angle.is_numerically_defined()) {
            os << fmt::format("{:.17f}", angle.numerical_);
        } else {
            switch (angle.numerator_) {
            case 1:
                break;
            case -1:
                os << '-';
                break;
            default:
                os << angle.numerator_ << '*';
                break;
            }
            os <<  "pi";
            if (angle.denominator_ != 1) {
                os <<  "/" << angle.denominator_;
            }
        }
        return os;
    }
#pragma endregion

private:
    constexpr void calculate_numerical_value()
    {
        double const factor = static_cast<double>(numerator_)
                              / static_cast<double>(denominator_);
        numerical_ = factor * PI_k;
    }

    constexpr void normalize()
    {
        if (is_numerically_defined()) {
            return;
        }
        if (numerator_ == 0) {
            denominator_ = 1;
        } else {
            int sign = 1;
            if (numerator_ < 0) {
                sign = -1;
                numerator_ = std::abs(numerator_);
            }
            if (denominator_ < 0) {
                sign *= -1;
                denominator_ = std::abs(denominator_);
            }
            auto tmp = std::gcd(numerator_, denominator_);
            numerator_ = (numerator_ / tmp) * sign;
            denominator_ = denominator_ / tmp;
        }
        if (denominator_ == 1) {
            numerator_ = (numerator_ % 2);
        }
        calculate_numerical_value();
    }

private:
    int32_t numerator_;
    int32_t denominator_;
    double numerical_;
};

namespace sym_angle {
/*! \brief identity */
constexpr Angle zero(0, 1);
/*! \brief rotation angle of a T gate */
constexpr Angle pi_quarter(1, 4);
/*! \brief rotation angle of a S gate (phase gate) */
constexpr Angle pi_half(1, 2);
/*! \brief rotation angle of a Pauli-Z gate, Pauli-X (NOT) */
constexpr Angle pi(1, 1);
} // namespace angles

} // namespace tweedledum
