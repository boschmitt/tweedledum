/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

namespace tweedledum {

namespace detail {

inline uint32_t sub_pattern(uint32_t num, uint32_t s, uint32_t e)
{
	return (num >> s) & ((1 << (e - s + 1)) - 1);
}

inline void transpose(std::vector<uint32_t>& matrix)
{
	// for 0 <= i < j < bit_count ...
	for (auto j = 1u; j < matrix.size(); ++j) {
		for (auto i = 0u; i < j; ++i) {
			auto mij = (matrix[i] >> j) & 1;
			auto mji = (matrix[j] >> i) & 1;

			if (mij == mji)
				continue;

			matrix[i] ^= (1 << j);
			matrix[j] ^= (1 << i);
		}
	}
}

inline std::vector<std::pair<uint16_t, uint16_t>>
lwr_cnot_synthesis(std::vector<uint32_t>& matrix, uint32_t n, uint32_t m)
{




	std::vector<std::pair<uint16_t, uint16_t>> gates;
	uint32_t sec_count = std::ceil(static_cast<float>(n) / m);
	for (auto sec = 0u; sec < sec_count; ++sec) {
		// remove duplicate sub-rows in section sec
		std::vector<uint32_t> patt(1 << m, (1<<n) );
		for (auto row = sec * m; row < n; ++row) {
			uint32_t start = sec * m;
			uint32_t end = start + m - 1;
			uint32_t sub_row_patt = sub_pattern(matrix[row], start, end);
			// if this is the first copy of pattern save it otherwise remove
			if (patt[sub_row_patt] == (1<<n))
				patt[sub_row_patt] = row;
			else {
				matrix[row] ^= matrix[patt[sub_row_patt]];
				gates.emplace_back(patt[sub_row_patt], row);
			}
		}
		// use Gaussian elimination for remaining entries in column section
		uint32_t temp = (sec==(sec_count-1)) ? n : ((sec + 1) * m);
		for (auto col = sec * m; col < temp ; col++) {
			// check for 1 on diagonal
			bool diag_one = (matrix[col] >> col) & 1;

			// remove ones in rows below column col
			for (auto row = col + 1; row < n; row++) {
				if (((matrix[row] >> col) & 1) == 0)
					continue;
				if (!diag_one) {
					matrix[col] ^= matrix[row];
					gates.emplace_back(row, col);
					diag_one = true;
				}

				matrix[row] ^= matrix[col];
				gates.emplace_back(col, row);
			}
		}
	}

	return gates;
}

} /* namespace detail */

template<class Network>
void cnot_patel(Network& net, std::vector<uint32_t>& matrix,
                uint32_t partition_size, std::vector<uint8_t> const& qubits_map)
{
	/* number of qubits can be taken from matrix, since it is n x n matrix. */
	const auto nqubits = matrix.size();
	std::vector<std::pair<uint16_t, uint16_t>> gates_u;
	std::vector<std::pair<uint16_t, uint16_t>> gates_l;

	gates_l = detail::lwr_cnot_synthesis(matrix, nqubits, partition_size);

	detail::transpose(matrix);
	gates_u = detail::lwr_cnot_synthesis(matrix, nqubits, partition_size);

	// if we were to explicitly swap, but we just swap in the for loop
	// std::for_each(gates_u.begin(), gates_u.end(), [](auto& g) {
	// std::swap(g.first, g.second);});

	std::reverse(gates_l.begin(), gates_l.end());
	for (const auto [c, t] : gates_u) {
		net.add_controlled_gate(gate_kinds_t::cx, qubits_map[t], qubits_map[c]);
	}
	for (const auto [c, t] : gates_l) {
		net.add_controlled_gate(gate_kinds_t::cx, qubits_map[c], qubits_map[t]);
	}
}

/*! \brief Linear circuit synthesis
 *
 * This algorithm is based on the work in [K.N. Patel, I.L. Markov, J.P. Hayes:
 * Optimal synthesis of linear reversible circuits, in QIC 8, 3&4, 282-294,
 * 2008.]
 *
 * The following code shows how to apply the algorithm to the example in the
 * original paper.
 *
   \verbatim embed:rst
   .. code-block:: c++
      dag_path<qc_gate> network = ...;
      std::vector<uint32_t> matrix{{0b000011,
                                    0b011001,
                                    0b010010,
                                    0b111111,
                                    0b111011,
                                    0b011100}};
      cnot_patel(network, matrix, 2);
   \endverbatim
 */
template<class Network>
void cnot_patel(Network& net, std::vector<uint32_t>& matrix,
                uint32_t partition_size)
{
	/* number of qubits can be taken from matrix, since it is n x n matrix. */
	const auto nqubits = matrix.size();
	for (auto i = 0u; i < nqubits; ++i) {
		net.allocate_qubit();
	}
	std::vector<uint8_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);

	cnot_patel(net, matrix, partition_size, qubits_map);
}

} // namespace tweedledum
