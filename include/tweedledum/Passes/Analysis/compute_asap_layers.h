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

inline std::vector<uint32_t> compute_asap_layers(Circuit const& circuit)
{
    std::vector<uint32_t> instruction_layer(circuit.size(), 0u);
    circuit.foreach_instruction([&](InstRef inst) {
        uint32_t layer = 0;
        circuit.foreach_child(inst, [&](InstRef child) {
            layer = std::max(layer, instruction_layer.at(child));
        });
        instruction_layer.at(inst) = ++layer;
    });
    // Correcting (I want layers to start from 0!)
    for (uint32_t& layer : instruction_layer) {
        layer -= 1;
    }
    return instruction_layer;
}

} // namespace tweedledum
