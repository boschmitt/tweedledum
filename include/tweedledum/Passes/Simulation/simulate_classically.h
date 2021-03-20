/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Wire.h"
#include "../../Operators/Reversible.h"
#include "../../Utils/DynamicBitset.h"

namespace tweedledum {

template<typename WordType>
inline DynamicBitset<WordType> simulate_classically(
    Circuit const& circuit, DynamicBitset<WordType> pattern)
{
    assert(circuit.num_qubits() == pattern.size());
    circuit.foreach_instruction([&](Instruction const& inst) {
        Qubit target = Qubit::invalid();
        bool execute = true;
        if (inst.is_a<Op::X>()) {
            inst.foreach_control([&](Qubit const& wire) {
                auto bit = pattern[wire];
                execute &= bit ^ wire.polarity();
            });
            target = inst.target();
        } else if (inst.is_a<Op::TruthTable>()) {
            auto const& tt = inst.cast<Op::TruthTable>();
            if (tt.is_phase()) {
                return;
            }
            uint32_t pos = 0u;
            for (uint32_t i = 0; i < inst.num_qubits() - 1; ++i) {
                Qubit const wire = inst.target(i);
                pos |= (pattern[wire] << i);
            }
            execute &= kitty::get_bit(tt.truth_table(), pos);
            target = inst.target(inst.num_qubits() - 1);
        }
        if (execute) {
            pattern.flip(target);
        }
    });
    return pattern;
}

} // namespace tweedledum
