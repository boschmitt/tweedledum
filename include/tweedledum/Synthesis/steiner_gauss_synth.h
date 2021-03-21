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

// This implementation is based on:
//
// Kissinger, Aleks, and Arianne Meijer-van de Griend. "CNOT circuit extraction
// for topologically-constrained quantum memories." 
// arXiv preprint arXiv:1904.00633 (2019).
//
// I also used staq's (https://github.com/softwareQinc/staq/) trick to deal 
// with 1's to the left of the diagonal
//
namespace tweedledum {

void steiner_gauss_synth(Circuit& circuit, Device const& device,
    BMatrix const& matrix, nlohmann::json const& config = {});

Circuit steiner_gauss_synth(Device const& device, BMatrix const& matrix,
    nlohmann::json const& config = {});

} // namespace tweedledum