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

// Pauli-Z operator
class Z {
    constexpr static std::array<Complex, 4> mat_ = {1., 0., 0., -1.};

public:
    static constexpr std::string_view kind()
    {
        return "std.z";
    }

    static Z adjoint()
    {
        return Z();
    }

    static double angle()
    {
        return numbers::pi;
    }

    static UMatrix2 const matrix()
    {
        return Eigen::Map<UMatrix2 const>(mat_.data());
    }
};

} // namespace tweedledum::Op
