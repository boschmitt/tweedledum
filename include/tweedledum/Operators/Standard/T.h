/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Matrix.h"
#include "../../Utils/Numbers.h"

#include <array>
#include <string_view>

namespace tweedledum::Op {

class Tdg;

// T operator.
//
// The operator induces a π/4 phase.  It is also known as the π/8 gate (because 
// of how the RZ(\pi/4) matrix looks like).  This is a non-Clifford gate and a
// fourth-root of Pauli-Z.
class T {
    constexpr static std::array<Complex, 4> mat_ =
        {1., 0., 0., {numbers::inv_sqrt2, numbers::inv_sqrt2}};
public:
    static constexpr std::string_view kind()
    {
        return "std.t";
    }

    static inline Tdg adjoint();

    static double angle() 
    {
        return numbers::pi_div_4;
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

// T dagger operator.
//
// The operator induces a -π/4 phase.  This is a non-Clifford gate and a 
// fourth-root of Pauli-Z.
class Tdg {
    constexpr static std::array<Complex, 4> mat_ =
        {1., 0., 0., {numbers::inv_sqrt2, -numbers::inv_sqrt2}};
public:
    static constexpr std::string_view kind()
    {
        return "std.tdg";
    }

    static T adjoint()
    {
        return T();
    }

    static double angle() 
    {
        return -numbers::pi_div_4;
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

Tdg T::adjoint()
{
    return Tdg();
}

} // namespace tweedledum
