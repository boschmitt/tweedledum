/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"

#include <kitty/kitty.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

// Synthesize a quantum circuit from a function by computing PPRM representation
//
// PPRM: The positive polarity Reed-Muller form is an ESOP, where each variable
// has positive polarity (not complemented form). PPRM is a canonical
// expression, so further minimization is not possible.
void pprm_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
  nlohmann::json const& config = {});

Circuit pprm_synth(kitty::dynamic_truth_table const& function,
  nlohmann::json const& config = {});

} // namespace tweedledum
