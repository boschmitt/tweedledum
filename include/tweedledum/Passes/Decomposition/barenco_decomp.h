/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

void barenco_decomp(Circuit& circuit, Instruction const& inst,
    nlohmann::json const& config = {});

void barenco_decomp(Circuit& circuit, Circuit const& original,
    nlohmann::json const& config = {});

Circuit barenco_decomp(Circuit const& original, 
    nlohmann::json const& config = {});

} // namespace tweedledum
