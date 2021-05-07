/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Instruction.h"

#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/networks/xag.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

// Synthesize a quantum circuit from a function by computing PKRM representation
// PKRM: Pseudo-Kronecker Read-Muller expression---a special case of an ESOP
// form.
void pkrm_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
  nlohmann::json const& config = {});

Circuit pkrm_synth(kitty::dynamic_truth_table const& function,
  nlohmann::json const& config = {});

} // namespace tweedledum
