/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../Target/Device.h"
#include "../Utils/Matrix.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

void sat_linear_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, BMatrix const& matrix,
  nlohmann::json const& config = {});

Circuit sat_linear_synth(
  BMatrix const& matrix, nlohmann::json const& config = {});

void sat_linear_synth(Device const& device, Circuit& circuit,
  BMatrix const& linear_trans, nlohmann::json const& config = {});

Circuit sat_linear_synth(Device const& device, BMatrix const& linear_trans,
  nlohmann::json const& config = {});

} // namespace tweedledum
