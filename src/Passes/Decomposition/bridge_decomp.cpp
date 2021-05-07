/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/bridge_decomp.h"

#include "tweedledum/Operators/Extension/Bridge.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

namespace {

inline bool decompose(
  Device const& device, Circuit& circuit, Instruction const& inst)
{
    std::vector<uint32_t> const path =
      device.shortest_path(inst.control(), inst.target());
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

} // namespace

void bridge_decomp(
  Device const& device, Circuit& circuit, Instruction const& inst)
{
    decompose(device, circuit, inst);
}

Circuit bridge_decomp(Device const& device, Circuit const& original)
{
    Circuit decomposed = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        if (inst.is_a<Op::Bridge>()) {
            decompose(device, decomposed, inst);
            return;
        }
        decomposed.apply_operator(inst);
    });
    return decomposed;
}

} // namespace tweedledum
