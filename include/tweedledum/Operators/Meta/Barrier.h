/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <string_view>

namespace tweedledum::Op {

class Barrier {
public:
    static constexpr std::string_view kind()
    {
        return "meta.barrier";
    }
};

} // namespace tweedledum
