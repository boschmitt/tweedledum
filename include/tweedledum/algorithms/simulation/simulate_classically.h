/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/DynamicBitset.h"

#include <algorithm>
#include <kitty/kitty.hpp>
#include <vector>

namespace tweedledum {

template<typename WordType>
inline DynamicBitset<WordType> simulate_classically(
    Circuit const& circuit, DynamicBitset<WordType> pattern)
{
	assert(circuit.num_qubits() == pattern.size());
	for (auto const& inst : circuit) {
		bool execute = true;
		if (inst.is<GateLib::TruthTable>()) {
			auto const& tt = inst.cast<GateLib::TruthTable>();
			// TODO: refactor:
			uint32_t i = 0;
			uint32_t pos = 0u;
			std::for_each(inst.begin(), inst.end() - 1,
			[&](WireRef const& wire) {
				pos |= (pattern[wire.uid()] << i);
				++i;
			});
			execute &= kitty::get_bit(tt.truth_table(), pos);
		} else if (inst.is<GateLib::X>()) {
			std::for_each(inst.begin(), inst.end() - 1,
			[&](WireRef const& wire) {
				auto bit = pattern[wire.uid()];
				execute &= bit ^ wire.polarity();
			});
		}
		if (execute) {
			pattern.flip(inst.target().uid());
		}
	}
	return pattern;
}

} // namespace tweedledum