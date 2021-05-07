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

inline std::vector<uint32_t> compute_alap_layers(Circuit const& circuit)
{
    std::vector<uint32_t> instruction_layer(circuit.size(), 0u);
    circuit.foreach_output(
      [&](InstRef const ref) { instruction_layer.at(ref) = 0u; });
    uint32_t max_layer = 0u;
    circuit.foreach_r_instruction([&](InstRef ref) {
        uint32_t layer = instruction_layer.at(ref) + 1;
        circuit.foreach_child(ref, [&](InstRef child) {
            instruction_layer.at(child) =
              std::max(instruction_layer.at(child), layer);
        });
        max_layer = std::max(max_layer, layer);
    });
    max_layer -= 1u;
    for (uint32_t& layer : instruction_layer) {
        layer = max_layer - layer;
    }
    return instruction_layer;
}

} // namespace tweedledum
