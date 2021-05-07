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
    inst.foreach_control([&](Qubit qubit) {
        circuit.apply_operator(Op::X(), {qubit, inst.target()}, inst.cbits());
    });
    return true;
}

} // namespace

void parity_decomp(Circuit& circuit, Instruction const& inst)
{
    decompose(circuit, inst);
}

Circuit parity_decomp(Circuit const& original)
{
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Parity>()) {
            decompose(decomposed, inst);
            return;
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
