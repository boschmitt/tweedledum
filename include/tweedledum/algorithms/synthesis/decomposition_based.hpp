/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/netlist.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <list>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `decomposition_based_synthesis`. */
struct decomposition_based_synthesis_params {
	/*! \brief Be verbose. */
	bool verbose{false};
};

namespace detail {

std::pair<std::vector<uint16_t>, std::vector<uint16_t>> decompose(std::vector<uint16_t>& perm,
                                                                  uint8_t var)
{
	std::vector<uint16_t> left(perm.size(), 0), right(perm.size(), 0);
	std::vector<bool> visited(perm.size(), false);

	uint16_t row{0u};

	while (true) {
		if (visited[row]) {
			const auto it = std::find(visited.begin(), visited.end(), false);
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
		                    std::find(perm.begin(), perm.end(), perm[row] ^ (1 << var)));
	}

	std::vector<uint16_t> perm_old = perm;
	for (uint32_t row = 0; row < perm.size(); ++row) {
		perm[left[row]] = right[perm_old[row]];
	}

	return {left, right};
}

std::pair<kitty::dynamic_truth_table, std::vector<uint32_t>>
control_function_abs(uint32_t num_vars, std::vector<uint16_t> const& perm)
{
	kitty::dynamic_truth_table tt(num_vars);
	for (uint32_t row = 0; row < perm.size(); ++row) {
		if (perm[row] != row) {
			kitty::set_bit(tt, row);
		}
	}

	std::vector<uint32_t> base;
	for (auto element : kitty::min_base_inplace(tt)) {
		base.push_back(element);
	}
	return {kitty::shrink_to(tt, base.size()), base};
}

} // namespace detail

/*! \brief Reversible synthesis based on functional decomposition.
 *
   \verbatim embed:rst
   This algorithm implements the decomposition-based synthesis algorithm
   proposed in :cite:`VR08`.  A permutation is specified as a vector of
   :math:`2^n` different integers ranging from :math:`0` to :math:`2^n-1`.

   .. code-block:: c++

      std::vector<uint16_t> perm{{0, 2, 3, 5, 7, 1, 4, 6}};
      auto circ = decomposition_based_synthesis<gg_network<mcst_gate>>(perm, stg_from_spectrum());
   \endverbatim
 *
 * \param perm Input permutation
 * \param stg_synth Synthesis function for single-target gates
 * \param ps Parameters
 * 
 * \algtype synthesis
 * \algexpects Permutation
 * \algreturns Quantum or reversible circuit
 */
template<class Network, class STGSynthesisFn>
Network decomposition_based_synthesis(std::vector<uint16_t>& perm, STGSynthesisFn&& stg_synth,
                                      decomposition_based_synthesis_params const& ps = {})
{
	Network circ;
	const uint32_t num_qubits = std::log2(perm.size());
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.add_qubit();
	}

	std::list<std::pair<kitty::dynamic_truth_table, std::vector<uint32_t>>> gates;
	auto pos = gates.begin();
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		const auto [left, right] = detail::decompose(perm, i);

		auto [tt_l, vars_l] = detail::control_function_abs(num_qubits, left);
		vars_l.push_back(i);

		auto [tt_r, vars_r] = detail::control_function_abs(num_qubits, right);
		vars_r.push_back(i);

		// TODO merge middle gates
		if (!kitty::is_const0(tt_l)) {
			pos = gates.emplace(pos, tt_l, vars_l);
			pos++;
		}
		if (!kitty::is_const0(tt_r)) {
			pos = gates.emplace(pos, tt_r, vars_r);
		}
	}

	for (auto const& [tt, vars] : gates) {
		if (ps.verbose)
			std::cout << "[i] synthesize " << kitty::to_hex(tt) << " onto "
			          << std::accumulate(vars.begin() + 1, vars.end(),
			                             std::to_string(vars.front()),
			                             [](auto const& a, auto v) {
				                             return a + ", " + std::to_string(v);
			                             })
			          << "\n";
		stg_synth(circ, tt, vars);
	}
	return circ;
}

} // namespace tweedledum
