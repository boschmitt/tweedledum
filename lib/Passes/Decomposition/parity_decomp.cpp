/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/parity_decomp.h"

#include "tweedledum/Operators/Extension.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

namespace {

inline bool decompose(Circuit& circuit, Instruction const& inst)
{
    inst.foreach_control([&](WireRef wref) {
        circuit.apply_operator(Op::X(), {wref, inst.target()});
    });
    return true;
}

}

void parity_decomp(Circuit& circuit, Instruction const& inst)
{
    decompose(circuit, inst);
}

Circuit parity_decomp(Circuit const& original)
{
    Circuit result = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Parity>()) {
            decompose(result, inst);
            return;
        }
        result.apply_operator(inst);
    });
    return result;
}

} // namespace tweedledum
