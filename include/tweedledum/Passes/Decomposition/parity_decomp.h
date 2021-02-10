/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

void parity_decomp(Circuit& circuit, Instruction const& inst);

Circuit parity_decomp(Circuit const& original);

} // namespace tweedledum
