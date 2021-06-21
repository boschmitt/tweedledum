/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/parity_decomp.h"

#include "tweedledum/Decomposition/ParityDecomposer.h"
#include "tweedledum/Operators/Extension/Parity.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

Circuit parity_decomp(Circuit const& original, nlohmann::json const& config)
{
    ParityDecomposer decomposer(config);
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Parity>()) {
            decomposer.decompose(decomposed, inst);
            return;
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
