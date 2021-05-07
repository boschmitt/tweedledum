/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"
#include "../Utils/LinPhasePoly.h"
#include "../Utils/Matrix.h"

#include <nlohmann/json.hpp>
#include <vector>

namespace tweedledum {

void cx_dihedral_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, BMatrix const& linear_trans,
  LinPhasePoly parities, nlohmann::json const& config = {});

Circuit cx_dihedral_synth(BMatrix const& linear_trans,
  LinPhasePoly const& phase_parities, nlohmann::json const& config = {});

} // namespace tweedledum
