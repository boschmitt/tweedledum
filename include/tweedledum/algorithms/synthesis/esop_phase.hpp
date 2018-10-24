/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/netlist.hpp"
#include "../../gates/mcmt_gate.hpp"

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <vector>

namespace tweedledum {

// TODO FIX THIS
template<typename Network>
void esop_phase_synthesis(Network& circ, kitty::dynamic_truth_table const& tt)
{
	const uint32_t num_qubits = tt.num_vars();
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.add_qubit();
	}
	for (const auto& cube : esop_from_pprm(tt)) {
		std::vector<uint32_t> controls;
		std::vector<uint32_t> targets;
		for (auto i = 0; i < tt.num_vars(); ++i) {
			if (!cube.get_mask(i)) {
				continue;
			}
			assert(cube.get_bit(i));
			if (targets.empty()) {
				targets.emplace_back(i);
			} else {
				controls.emplace_back(i);
			}
		}
		if (!targets.empty()) {
			circ.add_gate(gate_kinds_t::mcz, controls, targets);
		}
	}
}

}; // namespace tweedledum
