/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"
#include "compute_alap_layers.h"
#include "compute_asap_layers.h"

#include <vector>

namespace tweedledum {

inline std::vector<std::vector<InstRef>> compute_critical_paths(
  Circuit const& circuit)
{
    using Path = std::vector<InstRef>;
    std::vector<uint32_t> const alap_layer = compute_alap_layers(circuit);
    std::vector<uint32_t> const asap_layer = compute_asap_layers(circuit);
    std::vector<Path> critical_paths;
    circuit.foreach_output([&](InstRef const ref) {
        if (alap_layer.at(ref) == asap_layer.at(ref)) {
            critical_paths.emplace_back(1, ref);
        }
    });
    std::vector<uint32_t> added(alap_layer.size(), 0u);
    for (Path& path : critical_paths) {
        uint32_t current = 0u;
        do {
            circuit.foreach_child(path.at(current), [&](InstRef child) {
                if (added.at(child)
                    || alap_layer.at(child) != asap_layer.at(child)) {
                    return;
                }
                path.push_back(child);
                added.at(child) = 1u;
            });
            current += 1;
        } while (current < path.size());
        std::reverse(path.begin(), path.end());
        std::fill(added.begin(), added.end(), 0u);
    }
    return critical_paths;
}

} // namespace tweedledum
