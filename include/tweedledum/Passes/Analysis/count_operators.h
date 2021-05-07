/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace tweedledum {

inline auto count_operators(Circuit const& circuit)
{
    std::unordered_map<std::string, uint32_t> counters;
    circuit.foreach_instruction([&](Instruction const& inst) {
        std::string id =
          inst.num_controls() == 0
            ? fmt::format("{}", inst.name())
            : fmt::format("({}c){}", inst.num_controls(), inst.name());
        auto it = counters.find(id);
        if (it != counters.end()) {
            it->second += 1;
        }
        counters.emplace(id, 1u);
    });
    return counters;
}

} // namespace tweedledum
