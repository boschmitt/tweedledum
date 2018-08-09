/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

namespace tweedledum {

namespace detail {

uint32_t get_special_part_of_bin(uint32_t num, uint32_t s, uint32_t e,
                                 uint32_t bit_count)
{
	uint32_t res = num;
	uint32_t all_one = (1 << bit_count) - 1;
	uint32_t mask = all_one >> bit_count - (e - s + 1);

	res <<= bit_count - 1 - e;
	res >>= s + (bit_count - 1 - e);
	res &= mask;

	return res;
}

void transpose(std::vector<uint32_t>& matrix)
{
	uint32_t bit_count = matrix.size();

	// for 0 <= i < j < bit_count ...
	for (auto j = 1u, jr = bit_count - 2; j < bit_count; ++j, --jr) {
		for (auto i = 0u, ir = bit_count - 1; i < j; ++i, --ir) {
			auto mij = (matrix[i] >> jr) & 1;
			auto mji = (matrix[j] >> ir) & 1;

			if (mij == mji)
				continue;

			matrix[i] ^= (1 << jr);
			matrix[j] ^= (1 << ir);
		}
	}
}

std::vector<std::pair<uint16_t, uint16_t>>
lwr_cnot_synthesis(std::vector<uint32_t>& matrix, uint32_t n, uint32_t m)
{
	std::vector<std::pair<uint16_t, uint16_t>> gates;
	uint32_t sec_count = std::ceil(static_cast<float>(n) / m);
	for (auto sec = 1; sec <= sec_count; sec++) {
		// remove duplicate sub-rows in section sec
		std::vector<uint32_t> patt;
		for (auto i = 0; i < (1 << m); i++) {
			patt.push_back(-1);
		}
		for (auto row = (sec - 1) * m; row < n; row++) {
			uint32_t sub_row_patt;
			uint32_t temp = matrix[row];
			uint32_t start = (sec_count - sec) * m;
			uint32_t end = start + m - 1;
			sub_row_patt
			    = get_special_part_of_bin(temp, start, end, n);
			// if this is the first copy of pattern save it otherwise remove
			if (patt[sub_row_patt] == -1)
				patt[sub_row_patt] = row;
			else {
				matrix[row] ^= matrix[patt[sub_row_patt]];
				gates.push_back({patt[sub_row_patt], row});
			}
		}
		// use gaussian elimination for remaining entries in column section
		for (auto col = (sec - 1) * m; col <= sec * m - 1; col++) {
			// check for 1 on diagonal
			bool diag_one = true;
			uint32_t start = n - 1 - col;

			if (get_special_part_of_bin(matrix[col], start, start, n)
			    == 0)
				diag_one = false;
			// remove ones in rows below column col
			for (auto row = col + 1; row < n; row++) {
				uint32_t start = n - 1 - col;
				if (get_special_part_of_bin(matrix[row], start,
				                            start, n)
				    == 1) {

					if (!diag_one) {

						matrix[col] ^= matrix[row];
						gates.push_back({row, col});
						diag_one = true;
					}

					matrix[row] ^= matrix[col];

					gates.push_back({col, row});
				}
			}
		}
	}

	return gates;
}

} /* namespace detail */

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
      std::vector<uint32_t> matrix{{0b110000,
                                    0b100110,
                                    0b010010,
                                    0b111111,
                                    0b110111,
                                    001110}};
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
		net.add_controlled_gate(gate_kinds_t::cx, t, c);
	}
	for (const auto [c, t] : gates_l) {
		net.add_controlled_gate(gate_kinds_t::cx, c, t);
	}
}

} // namespace tweedledum
