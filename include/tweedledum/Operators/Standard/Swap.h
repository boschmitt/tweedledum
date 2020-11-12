/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Matrix.h"

#include <string_view>

namespace tweedledum::Op {

class Swap {
public:
    static constexpr std::string_view kind()
    {
        return "std.swap";
    }

    static Swap adjoint()
    {
        return Swap();
    }

    UMatrix4 const matrix() const
    {
        return (UMatrix4() << 1, 0 ,0, 0,
                              0, 0, 1, 0,
                              0, 1, 0, 0,
                              0, 0, 0, 1).finished();
    }

    uint32_t num_targets() const
    {
        return 2u;
    }
};

} // namespace tweedledum
