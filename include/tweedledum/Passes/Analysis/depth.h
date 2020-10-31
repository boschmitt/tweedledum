/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"

#include <algorithm>
#include <vector>

namespace tweedledum {

inline uint32_t depth(Circuit const& circuit)
{
    uint32_t depth = 0u;
    std::vector<uint32_t> layer(circuit.size(), 0u);
    circuit.foreach_instruction([&](InstRef inst) {
        uint32_t l = 0;
        circuit.foreach_child(inst, [&](InstRef child) {
            l = std::max(l, layer.at(child));
        });
        layer.at(inst) = ++l;
        depth = std::max(depth, l);
    });
    return depth;
}

} // namespace tweedledum
