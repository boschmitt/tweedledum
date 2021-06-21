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
    // This is weird:
    // There is a GCC bug that prevents me from using [[maybe_unused]] in the 
    // begining for the first argument.  
    //      https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81429
    // As far as I know, this bug was only fixed on GCC 9.3, but to build wheels
    // the image uses GCC 8
    ParityDecomposer(nlohmann::json const& config [[maybe_unused]] = {})
    {}

    bool decompose(Circuit& circuit, Instruction const& inst);
};
} // namespace tweedledum
