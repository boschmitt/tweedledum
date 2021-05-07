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
    std::vector<uint32_t> to_remove(original.size(), 0u);
    std::vector<InstRef> qubit_last(original.num_qubits(), InstRef::invalid());
    std::vector<InstRef> cbit_last(original.num_cbits(), InstRef::invalid());
    original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        // Check children
        InstRef temp = qubit_last.at(inst.qubit(0u));
        bool all_equal = false;
        inst.foreach_qubit([&](Qubit const qubit) {
            all_equal = (temp == qubit_last.at(qubit));
        });
        inst.foreach_cbit(
          [&](Cbit const cbit) { all_equal = (temp == cbit_last.at(cbit)); });
        if (!all_equal || temp == InstRef::invalid()) {
            inst.foreach_qubit(
              [&](Qubit const qubit) { qubit_last.at(qubit) = ref; });
            inst.foreach_cbit(
              [&](Cbit const cbit) { cbit_last.at(cbit) = ref; });
            return;
        }
        Instruction const& other = original.instruction(temp);
        if (!inst.is_adjoint(other)) {
            inst.foreach_qubit(
              [&](Qubit const qubit) { qubit_last.at(qubit) = ref; });
            inst.foreach_cbit(
              [&](Cbit const cbit) { cbit_last.at(cbit) = ref; });
            return;
        }
        to_remove.at(temp) = 1u;
        to_remove.at(ref) = 1u;
        other.foreach_qubit([&](Qubit const qubit, InstRef child) {
            qubit_last.at(qubit) = child;
        });
        other.foreach_cbit(
          [&](Cbit const cbit, InstRef child) { cbit_last.at(cbit) = child; });
    });

    // Remove marked gates:
    Circuit result = shallow_duplicate(original);
    original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        if (to_remove.at(ref)) {
            return;
        }
        result.apply_operator(inst);
    });
    return result;
}

} // namespace tweedledum
