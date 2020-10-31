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
        bool execute = true;
        if (inst.is_a<Op::X>()) {
            inst.foreach_control([&](WireRef const& wire) {
                auto bit = pattern[wire];
                execute &= bit ^ wire.polarity();
            });
        } else if (inst.is_a<Op::TruthTable>()) {
            auto const& tt = inst.cast<Op::TruthTable>();
            // TODO: refactor:
            uint32_t i = 0;
            uint32_t pos = 0u;
            inst.foreach_control([&] (WireRef const& wire) {
                pos |= (pattern[wire] << i);
                ++i;
            });
            execute &= kitty::get_bit(tt.truth_table(), pos);
        }
        if (execute) {
            pattern.flip(inst.target());
        }
    });
    return pattern;
}

} // namespace tweedledum
