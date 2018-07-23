/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../representations/netlist.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <list>
#include <vector>

namespace tweedledum {
namespace detail {

std::pair<std::vector<uint16_t>, std::vector<uint16_t>>
decompose(std::vector<uint16_t>& perm, uint8_t var)
{
	std::vector<uint16_t> left(perm.size(), 0), right(perm.size(), 0);
	std::vector<bool> visited(perm.size(), false);

	uint16_t row{0u};

	while (true) {
		if (visited[row]) {
			const auto it
			    = std::find(visited.begin(), visited.end(), false);
			if (it == visited.end()) {
				break;
			}
			row = std::distance(visited.begin(), it);
		}

		/* assign 0 to var on left side */
		left[row] = (row & ~(1 << var));
		visited[row] = true;

		/* assign 1 to var on left side */
		left[row ^ (1 << var)] = left[row] ^ (1 << var);
		row ^= (1 << var);
		visited[row] = true;

		/* assign 1 to var on right side */
		right[perm[row] | (1 << var)] = perm[row];

		/* assign 0 to var on left side */
		right[perm[row] & ~(1 << var)] = perm[row] ^ (1 << var);

		row = std::distance(perm.begin(),
		                    std::find(perm.begin(), perm.end(),
		                              perm[row] ^ (1 << var)));
	}

	std::vector<uint16_t> perm_old = perm;
	for (uint32_t row = 0; row < perm.size(); ++row) {
		perm[left[row]] = right[perm_old[row]];
	}

	return {left, right};
}

kitty::dynamic_truth_table
control_function_abs(uint32_t num_vars, std::vector<uint16_t> const& perm)
{
	kitty::dynamic_truth_table tt(num_vars);
	for (uint32_t row = 0; row < perm.size(); ++row) {
		if (perm[row] != row) {
			kitty::set_bit(tt, row);
		}
	}
	return tt;
}

} // namespace detail

template<typename Network>
void decomposition_based_synthesis(Network& circ, std::vector<uint16_t>& perm)
{
	const uint32_t num_qubits = std::log2(perm.size());
	// netlist circ(num_qubits);
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.allocate_qubit();
	}
	std::list<std::pair<uint16_t, uint16_t>> gates;
	auto pos = gates.begin();
	for (uint8_t i = 0u; i < num_qubits; ++i) {
		const auto [left, right] = detail::decompose(perm, i);

		for (const auto& cube : esop_from_pprm(
		         detail::control_function_abs(num_qubits, left))) {
			assert(((cube._bits >> i) & 1) == 0);
			pos = gates.emplace(pos, cube._bits, 1 << i);
			pos++;
		}

		for (const auto& cube : esop_from_pprm(
		         detail::control_function_abs(num_qubits, right))) {
			assert(((cube._bits >> i) & 1) == 0);
			pos = gates.emplace(pos, cube._bits, 1 << i);
		}
	}
	for (const auto [c, t] : gates) {
		circ.add_toffoli(c, t);
	}
	return circ;
}

}; // namespace tweedledum
