/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Decomposition/BridgeDecomposer.h"

#include "tweedledum/Operators/Extension/Bridge.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

bool BridgeDecomposer::decompose(Circuit& circuit, Instruction const& inst)
{
    assert(inst.is_a<Op::Bridge>());
    std::vector<uint32_t> const path =
      device_.shortest_path(inst.control(), inst.target());
    assert(path.size() >= 3u);
    for (uint32_t i = 1; i < path.size() - 1; ++i) {
        circuit.apply_operator(
          Op::X(), {Qubit(path.at(i)), Qubit(path.at(i + 1))});
    }
    for (uint32_t i = path.size() - 2; i-- > 1;) {
        circuit.apply_operator(
          Op::X(), {Qubit(path.at(i)), Qubit(path.at(i + 1))});
    }
    for (uint32_t i = 0; i < path.size() - 1; ++i) {
        circuit.apply_operator(
          Op::X(), {Qubit(path.at(i)), Qubit(path.at(i + 1))});
    }
    for (uint32_t i = path.size() - 2; i-- > 0;) {
        circuit.apply_operator(
          Op::X(), {Qubit(path.at(i)), Qubit(path.at(i + 1))});
    }
    return true;
}

} // namespace tweedledum
