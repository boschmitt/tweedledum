/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "shallow_duplicate.h"

namespace tweedledum {

inline Circuit reverse(Circuit const& original)
{
    Circuit reversed = shallow_duplicate(original);
    original.foreach_r_instruction([&](Instruction const& inst) {
        reversed.apply_operator(inst);
    });
    return reversed;
}

} // namespace tweedledum
