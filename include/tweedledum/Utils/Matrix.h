/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <complex>
#include <Eigen/Dense>
#include <iostream>

namespace tweedledum {

// Since Eigen has not concept of Boolean matrix with XOR, I'm implementing 
// my own Boolean type where "+" is a XOR.  It's kind of a hack, but well...
class MyBool {
public:
    constexpr MyBool()
        : value_(false)
    {}

    constexpr MyBool(bool value)
        : value_(value)
    {}

    constexpr MyBool(int value)
        : value_(value != 0)
    {}

    constexpr MyBool(uint32_t value)
        : value_(value != 0)
    {}

    bool operator==(MyBool const other) const
    {
        return other.value_ == value_;
    }

    MyBool operator+(MyBool const other) const
    {
        return other.value_ ^ value_;
    } 

    MyBool operator+=(MyBool const other)
    {
        value_ ^= other.value_;
        return value_;
    }

    operator uint32_t() const
    {
        return static_cast<uint32_t>(value_);
    }

    friend std::ostream& operator<<(std::ostream& os, MyBool const& value);

private:
    bool value_;
};

inline std::ostream& operator<<(std::ostream& os, MyBool const& value)
{
    os << value.value_;
    return os; 
}

} // namespace tweedledum

namespace Eigen {

template<>
struct NumTraits<tweedledum::MyBool> {
    typedef int Real;
    typedef tweedledum::MyBool Nested;
    typedef uint32_t Literal;
    enum {
        IsComplex = 0,
        IsInteger = 1,
        IsSigned = 0,
        RequireInitialization = 0,
        ReadCost = 1,
        AddCost = 2,
        MulCost = 2
    };
    // Maybe 1? (I'm not sure)
    static Real epsilon() { return 0; }
    static Real digits10() { return 1; }
    static Real dummy_precision() { return 0; }
};

} // namespace Eigen

namespace tweedledum {

using BMatrix = Eigen::Matrix<MyBool, Eigen::Dynamic, Eigen::Dynamic>;

using Complex = std::complex<double>;

// Unitary matrix is Column-Major!
using UMatrix = Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic>;
using UMatrix2 = Eigen::Matrix<Complex, 2, 2>;
using UMatrix4 = Eigen::Matrix<Complex, 4, 4>;

} // namespace tweedledum
