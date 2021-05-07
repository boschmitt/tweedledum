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

class Sdg;

// S operator.
//
// The operator induces a π/2 phase. This is a Clifford gate and a square-root
// of Pauli-Z.
class S {
    constexpr static std::array<Complex, 4> mat_ = {1., 0., 0., {0., 1.}};

public:
    static constexpr std::string_view kind()
    {
        return "std.s";
    }

    static inline Sdg adjoint();

    static double angle()
    {
        return numbers::pi_div_2;
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

class Sdg {
    constexpr static std::array<Complex, 4> mat_ = {1., 0., 0., {0., -1.}};

public:
    static constexpr std::string_view kind()
    {
        return "std.sdg";
    }

    static S adjoint()
    {
        return S();
    }

    static double angle()
    {
        return -numbers::pi_div_2;
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

Sdg S::adjoint()
{
    return Sdg();
}

} // namespace tweedledum::Op
