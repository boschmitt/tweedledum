/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Wire.h"

#include <kitty/kitty.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

void spectrum_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    kitty::dynamic_truth_table const& function, nlohmann::json const& config);

Circuit spectrum_synth(kitty::dynamic_truth_table const& function, nlohmann::json const& config);

} // namespace tweedledum
