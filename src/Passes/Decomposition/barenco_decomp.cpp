/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/barenco_decomp.h"

#include "tweedledum/Decomposition/BarencoDecomposer.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Operators/Standard/Y.h"
#include "tweedledum/Operators/Standard/Z.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

Circuit barenco_decomp(Circuit const& original, nlohmann::json const& config)
{
    BarencoDecomposer decomposer(config);
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_one<Op::X, Op::Y, Op::Z>()) {
            if (inst.num_controls() > decomposer.config.controls_threshold) {
                decomposer.decompose(decomposed, inst);
                return;
            }
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
