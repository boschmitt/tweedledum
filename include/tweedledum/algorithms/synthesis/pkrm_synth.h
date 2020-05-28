/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"

#include <cassert>
#include <kitty/kitty.hpp>
#include <vector>

namespace tweedledum {

using TruthTable = kitty::dynamic_truth_table;

inline void pkrm_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    TruthTable const& function)
{
	uint32_t const num_controls = function.num_vars();
	assert(qubits.size() >= (num_controls + 1u));

	WireRef const target = qubits.back();
	std::vector<WireRef> controls;
	controls.reserve(num_controls);
	for (auto const& cube : kitty::esop_from_optimum_pkrm(function)) {
		auto bits = cube._bits;
		auto mask = cube._mask;
		for (uint32_t v = 0u; mask; mask >>= 1, bits >>= 1, ++v) {
			if ((mask & 1) == 0u) {
				continue;
			}
			controls.push_back(
			    (bits & 1) ? qubits.at(v) : !qubits.at(v));
		}
		circuit.create_instruction(GateLib::X{}, controls, target);
		controls.clear();
	}
}

inline Circuit pkrm_synth(TruthTable const& function)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");
	// Create the necessary qubits
	std::vector<WireRef> wires;
	wires.reserve(function.num_vars());
	for (uint32_t i = 0u; i < function.num_vars() + 1; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	pkrm_synth(circuit, wires, function);
	return circuit;
}

} // namespace tweedledum