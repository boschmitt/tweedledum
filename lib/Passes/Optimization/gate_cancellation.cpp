/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Optimization/gate_cancellation.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

#include <vector>

namespace tweedledum {

Circuit gate_cancellation(Circuit const& original)
{
    Circuit result = shallow_duplicate(original);
    std::vector<uint32_t> visited(original.size(), 0);
    original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        std::vector<InstRef> children;
        original.foreach_child(ref, [&](InstRef const cref) {
            do {
                if (visited.at(cref) == 1) {

                }
                // if (op.is_adjoint(ancestor.op) || op.is_dependent(ancestor.op)) {
                //     children.emplace_back(circuit.id(ancestor));
                //     return;
                // }
            } while (1);
        });
        // Check children
        // Do remove
    });
    return result;
}

} // namespace tweedledum
