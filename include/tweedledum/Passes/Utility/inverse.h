/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "shallow_duplicate.h"

#include <optional>

namespace tweedledum {

inline std::optional<Circuit> inverse(Circuit const& original)
{
    Circuit adjoint_circuit = shallow_duplicate(original);
    bool failed = false;
    original.foreach_r_instruction([&](Instruction const& inst) {
        auto adjoint_op = inst.adjoint();
        if (!adjoint_op) {
            failed = true;
            return;
        }
        adjoint_circuit.apply_operator(
          *adjoint_op, inst.qubits(), inst.cbits());
    });
    if (failed) {
        return std::nullopt;
    }
    return adjoint_circuit;
}

} // namespace tweedledum
