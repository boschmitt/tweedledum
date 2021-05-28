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

class Sxdg;

// Sqrt(X) operator.
class Sx {
    constexpr static std::array<Complex, 4> mat_ = {Complex(0.5, 0.5),
      Complex(0.5, -0.5), Complex(0.5, -0.5), Complex(0.5, 0.5)};

public:
    static constexpr std::string_view kind()
    {
        return "std.sx";
    }

    static inline Sxdg adjoint();

    static double angle()
    {
        return numbers::pi_div_4;
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

class Sxdg {
    constexpr static std::array<Complex, 4> mat_ = {Complex(0.5, -0.5),
      Complex(0.5, 0.5), Complex(0.5, 0.5), Complex(0.5, -0.5)};

public:
    static constexpr std::string_view kind()
    {
        return "std.sxdg";
    }

    static Sx adjoint()
    {
        return Sx();
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

Sxdg Sx::adjoint()
{
    return Sxdg();
}

} // namespace tweedledum::Op
