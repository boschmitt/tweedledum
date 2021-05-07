/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../Target/Device.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

Circuit a_star_swap_synth(Device const& device,
  std::vector<uint32_t> const& init_cfg, std::vector<uint32_t> const& final_cfg,
  nlohmann::json const& config = {});

} // namespace tweedledum