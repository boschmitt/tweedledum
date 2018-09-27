/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../traits.hpp"
#include "../../networks/netlist.hpp"

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <vector>

namespace tweedledum {

template<typename Network>
void esop_based_synthesis(Network& circ, kitty::dynamic_truth_table const& tt)
{
	const uint32_t num_qubits = tt.num_vars() + 1;
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.add_qubit();
	}
	uint32_t target = tt.num_vars();
	std::cout << target << '\n';
	for (const auto& cube : kitty::esop_from_pprm(tt)) {
		std::vector<uint32_t> controls;
		for (auto i = cube._bits, j = 0u; i; i >>= 1, ++j) {
			if (i & 1) {
				controls.push_back(j);
			}
		}
		circ.add_gate(gate_kinds_t::mcx, controls, std::vector({target}));
	}
}

}; // namespace tweedledum
