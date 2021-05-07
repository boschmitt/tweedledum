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

// Hadamard operator.
//
// The operator is a π rotation about the X+Z axis, and has the effect of
// changing computation basis from |0〉(|1〉) to |+〉(|-〉) and vice-versa.
class H {
    constexpr static std::array<Complex, 4> mat_ = {numbers::inv_sqrt2,
      numbers::inv_sqrt2, numbers::inv_sqrt2, -numbers::inv_sqrt2};

public:
    static constexpr std::string_view kind()
    {
        return "std.h";
    }

    static H adjoint()
    {
        return H();
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

} // namespace tweedledum::Op
