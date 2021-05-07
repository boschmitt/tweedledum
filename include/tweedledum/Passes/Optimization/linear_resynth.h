/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

Circuit linear_resynth(
  Circuit const& original, nlohmann::json const& config = {});

} // namespace tweedledum
