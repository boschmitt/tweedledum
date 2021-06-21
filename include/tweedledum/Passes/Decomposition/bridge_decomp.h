/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../Target/Device.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

Circuit bridge_decomp(Device const& device, Circuit const& original,
  nlohmann::json const& config = {});

} // namespace tweedledum
