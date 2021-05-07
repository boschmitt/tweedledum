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

// This operator only makes sense when dealing with mapped circuits.
// A `Bridge` operator is basically a fancy `X`.  Functionally they behave
// in the exact same way.  However, Bridge does not requires its qubits to
// be connected, because its decomposition should take into account the
// device coupling constraints.
class Bridge {
    constexpr static std::array<Complex, 4> mat_ = {0., 1., 1., 0.};

public:
    static constexpr std::string_view kind()
    {
        return "ext.bridge";
    }

    static Bridge adjoint()
    {
        return Bridge();
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
