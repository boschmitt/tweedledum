/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/LinearPP.h"
#include "gray_synth.h"
#include "all_linear_synth.h"

#include <cassert>
#include <kitty/kitty.hpp>
#include <vector>

namespace tweedledum {

using TruthTable = kitty::dynamic_truth_table;

inline void spectrum_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    TruthTable const& function)
{
	uint32_t const num_controls = function.num_vars();
	assert(qubits.size() >= (num_controls + 1u));

	auto extended_f = kitty::extend_to(function, num_controls + 1);
	auto g = extended_f.construct();
	kitty::create_nth_var(g, num_controls);
	extended_f &= g;

	LinearPP parities;
	uint32_t const norm = (1 << extended_f.num_vars());
	auto const spectrum = kitty::rademacher_walsh_spectrum(extended_f);
	for (uint32_t i = 1u; i < spectrum.size(); ++i) {
		if (spectrum[i] == 0) {
			continue;
		}
		parities.add_term(i, norm * spectrum[i]);
	}
	circuit.create_instruction(GateLib::H(), {qubits.back()});
	if (parities.size() == spectrum.size() - 1) {
		all_linear_synth(circuit, qubits, parities);
	} else {
		gray_synth(circuit, qubits,
		    Matrix<uint8_t>::Identity(qubits.size()), parities);
	}
	circuit.create_instruction(GateLib::H(), {qubits.back()});
}

inline Circuit spectrum_synth(TruthTable const& function)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");
	// Create the necessary qubits
	std::vector<WireRef> wires;
	wires.reserve(function.num_vars());
	for (uint32_t i = 0u; i < function.num_vars() + 1; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	spectrum_synth(circuit, wires, function);
	return circuit;
}

} // namespace tweedledum