/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"
#include "../../Target/Device.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

void bridge_decomp(
  Device const& device, Circuit& circuit, Instruction const& inst);

Circuit bridge_decomp(Device const& device, Circuit const& original);

} // namespace tweedledum
