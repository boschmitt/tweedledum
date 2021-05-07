/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"

#include <mockturtle/networks/xag.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

void lhrs_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, mockturtle::xag_network const& xag,
  nlohmann::json const& config = {});

//  LUT-based hierarchical reversible logic synthesis (LHRS)
Circuit lhrs_synth(
  mockturtle::xag_network const& xag, nlohmann::json const& config = {});

} // namespace tweedledum
