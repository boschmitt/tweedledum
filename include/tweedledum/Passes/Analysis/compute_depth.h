/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "compute_asap_layers.h"

#include <algorithm>
#include <vector>

namespace tweedledum {

inline uint32_t compute_depth(Circuit const& circuit)
{
    std::vector<uint32_t> layers = compute_asap_layers(circuit);
    auto it = std::max_element(layers.begin(), layers.end());
    return *it + 1;
}

} // namespace tweedledum
