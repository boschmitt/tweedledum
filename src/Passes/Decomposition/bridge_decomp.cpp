/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/bridge_decomp.h"

#include "tweedledum/Decomposition/BridgeDecomposer.h"
#include "tweedledum/Operators/Extension/Bridge.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

Circuit bridge_decomp(
  Device const& device, Circuit const& original, nlohmann::json const& config)
{
    BridgeDecomposer decomposer(device, config);
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Bridge>()) {
            decomposer.decompose(decomposed, inst);
            return;
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
