/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Instruction.h"
#include "../Target/Device.h"

#include <nlohmann/json.hpp>

namespace tweedledum {

class BridgeDecomposer {
public:
    BridgeDecomposer(
      Device const& device, [[maybe_unused]] nlohmann::json const& config = {})
        : device_(device)
    {}

    bool decompose(Circuit& circuit, Instruction const& inst);

private:
    Device const& device_;
};

} // namespace tweedledum
