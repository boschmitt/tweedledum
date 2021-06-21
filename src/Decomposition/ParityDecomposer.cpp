/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Decomposition/ParityDecomposer.h"

#include "tweedledum/Operators/Extension/Parity.h"
#include "tweedledum/Operators/Standard/X.h"

namespace tweedledum {

bool ParityDecomposer::decompose(Circuit& circuit, Instruction const& inst)
{
    assert(inst.is_a<Op::Parity>());
    inst.foreach_control([&](Qubit qubit) {
        circuit.apply_operator(Op::X(), {qubit, inst.target()}, inst.cbits());
    });
    return true;
}

} // namespace tweedledum
