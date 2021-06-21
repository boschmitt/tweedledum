/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Instruction.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

class ParityDecomposer {
public:
    ParityDecomposer([[maybe_unused]] nlohmann::json const& config = {})
    {}

    bool decompose(Circuit& circuit, Instruction const& inst);
};
} // namespace tweedledum
