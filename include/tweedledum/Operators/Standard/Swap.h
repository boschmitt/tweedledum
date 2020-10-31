/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <string_view>

namespace tweedledum::Op {

class Swap {
public:
    static constexpr std::string_view kind()
    {
        return "std.swap";
    }

    uint32_t num_targets() const
    {
        return 2u;
    }
};

} // namespace tweedledum
