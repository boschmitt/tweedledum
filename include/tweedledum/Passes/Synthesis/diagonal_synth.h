/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Wire.h"
#include "../../Utils/Angle.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

void diagonal_synth(Circuit& circuit, std::vector<WireRef> qubits,
    std::vector<Angle> const& angles, nlohmann::json const& config);

Circuit diagonal_synth(std::vector<Angle> const& angles, nlohmann::json const& config);

} // namespace tweedledum
