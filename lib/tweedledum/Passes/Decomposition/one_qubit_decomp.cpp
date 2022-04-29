/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
// At this point this is basically a C++ implementaiton of what you see in:
// https://github.com/Qiskit/qiskit-terra/blob/main/qiskit/quantum_info/synthesis/one_qubit_decompose.py

#include "tweedledum/Passes/Decomposition/one_qubit_decomp.h"

#include "tweedledum/Decomposition/OneQubitDecomposer.h"
#include "tweedledum/Operators/Extension/Unitary.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

#include <cmath>

namespace tweedledum {

Circuit one_qubit_decomp(Circuit const& original, nlohmann::json const& config)
{
    OneQubitDecomposer decomposer(config);
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Unitary>() && inst.num_qubits() == 1u) {
            decomposer.decompose(decomposed, inst);
            return;
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
