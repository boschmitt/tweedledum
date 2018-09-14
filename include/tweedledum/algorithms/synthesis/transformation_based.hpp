/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/netlist.hpp"

#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <kitty/detail/mscfix.hpp>
#include <vector>

namespace tweedledum {

void update_permutation(std::vector<uint16_t>& perm, uint16_t controls,
                        uint16_t targets)
{
	std::for_each(perm.begin(), perm.end(), [&](auto& z) {
		if ((z & controls) == controls) {
			z ^= targets;
		}
	});
}

void update_permutation_inv(std::vector<uint16_t>& perm, uint16_t controls,
                            uint16_t targets)
{
	for (auto i = 0u; i < perm.size(); ++i) {
		if ((i & controls) != controls)
			continue;
		if (const auto partner = i ^ targets; partner > i) {
			std::swap(perm[i], perm[partner]);
		}
	}
}

template<typename Network>
void transformation_based_synthesis(Network& circ, std::vector<uint16_t>& perm)
{
	const uint32_t num_qubits = std::log2(perm.size());
	// netlist circ(num_qubits);
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.allocate_qubit();
	}
	std::vector<std::pair<uint16_t, uint16_t>> gates;
	uint32_t x{0u}; /* we need 32-bit if num_vars == 16 */
	while (x < perm.size()) {
		/* skip identity lines */
		auto y = perm[x];
		if (y == x) {
			++x;
			continue;
		}

		/* move 0s to 1s */
		if (const uint16_t t01 = x & ~y) {
			update_permutation(perm, y, t01);
			gates.emplace_back(y, t01);
		}

		/* move 1s to 0s */
		if (const uint16_t t10 = ~x & y) {
			update_permutation(perm, x, t10);
			gates.emplace_back(x, t10);
		}

		++x;
	}
	std::reverse(gates.begin(), gates.end());
	for (const auto [c, t] : gates) {
		circ.add_multiple_controlled_target_gate(gate_kinds_t::mcx, c, t);
	}
}

template<typename Network>
void transformation_based_synthesis_bidirectional(Network& circ,
                                                  std::vector<uint16_t>& perm)
{
	const uint32_t num_qubits = std::log2(perm.size());
	// netlist circ(num_qubits);
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.allocate_qubit();
	}
	std::list<std::pair<uint16_t, uint16_t>> gates;
	auto pos = gates.begin();
	uint32_t x{0u}; /* we need 32-bit if num_vars == 16 */
	while (x < perm.size()) {
		/* skip identity lines */
		auto y = perm[x];
		if (y == x) {
			++x;
			continue;
		}
		const uint16_t xs = std::distance(
		    perm.begin(), std::find(perm.begin() + x, perm.end(), x));

		if (__builtin_popcount(x ^ y) <= __builtin_popcount(x ^ xs)) {
			/* move 0s to 1s */
			if (const uint16_t t01 = x & ~y) {
				update_permutation(perm, y, t01);
				pos = gates.emplace(pos, y, t01);
			}
			/* move 1s to 0s */
			if (const uint16_t t10 = ~x & y) {
				update_permutation(perm, x, t10);
				pos = gates.emplace(pos, x, t10);
			}
		} else {
			/* move 0s to 1s */
			if (const uint16_t t01 = ~xs & x) {
				update_permutation_inv(perm, xs, t01);
				pos = gates.emplace(pos, xs, t01);
				pos++;
			}
			/* move 1s to 0s */
			if (const uint16_t t10 = xs & ~x) {
				update_permutation_inv(perm, x, t10);
				pos = gates.emplace(pos, x, t10);
				pos++;
			}
		}

		++x;
	}
	for (const auto [c, t] : gates) {
		circ.add_multiple_controlled_target_gate(gate_kinds_t::mcx, c, t);
	}
}

template<typename Network>
void transformation_based_synthesis_multidirectional(
    Network& circ, std::vector<uint16_t>& perm)
{
	const uint32_t num_qubits = std::log2(perm.size());
	// netlist circ(num_qubits);
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.allocate_qubit();
	}
	/* cost function (x is current input pattern, z is possible candidate)
	 */
	const auto cost_f = [&](auto x, auto z) {
		/* hamming distance from z to x and from x to f(z) */
		return __builtin_popcount(z ^ x)
		       + __builtin_popcount(x ^ perm[z]);
	};

	std::list<std::pair<uint16_t, uint16_t>> gates;
	auto pos = gates.begin();
	uint32_t x{0u}; /* we need 32-bit if num_vars == 16 */
	while (x < perm.size()) {
		/* find cheapest assignment */
		auto x_best = x;
		auto x_cost = __builtin_popcount(x ^ perm[x]);
		for (auto xx = x + 1; xx < perm.size(); ++xx) {
			if (const auto cost = cost_f(x, xx); cost < x_cost) {
				x_best = xx;
				x_cost = cost;
			}
		}

		const auto z = x_best;
		const auto y = perm[z];

		/* map z |-> x */
		/* move 0s to 1s */
		if (const uint16_t t01 = ~z & x) {
			update_permutation_inv(perm, z, t01);
			pos = gates.emplace(pos, z, t01);
			pos++;
		}

		/* move 1s to 0s */
		if (const uint16_t t10 = z & ~x) {
			update_permutation_inv(perm, x, t10);
			pos = gates.emplace(pos, x, t10);
			pos++;
		}

		/* map y |-> x */
		/* move 0s to 1s */
		if (const uint16_t t01 = x & ~y) {
			update_permutation(perm, y, t01);
			pos = gates.emplace(pos, y, t01);
		}

		/* move 1s to 0s */
		if (const uint16_t t10 = ~x & y) {
			update_permutation(perm, x, t10);
			pos = gates.emplace(pos, x, t10);
		}

		++x;
	}
	for (const auto [c, t] : gates) {
		circ.add_multiple_controlled_target_gate(gate_kinds_t::mcx, c, t);
	}
}

}; // namespace tweedledum
