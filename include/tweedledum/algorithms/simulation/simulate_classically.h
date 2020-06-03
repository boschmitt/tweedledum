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

using TruthTable = kitty::dynamic_truth_table;

template<typename WordType>
inline DynamicBitset<WordType> simulate_classically(
    Circuit const& circuit, DynamicBitset<WordType> pattern)
{
	assert(circuit.num_qubits() == pattern.size());
	for (auto const& inst : circuit) {
		assert(inst.kind() == "x");
		bool execute = true;
		std::for_each(
		    inst.begin(), inst.end() - 1, [&](WireRef const& wire) {
			    auto bit = pattern[wire.uid()];
			    execute &= bit ^ wire.polarity();
		    });
		if (execute) {
			pattern.flip(inst.target().uid());
		}
	}
	return pattern;
}

} // namespace tweedledum