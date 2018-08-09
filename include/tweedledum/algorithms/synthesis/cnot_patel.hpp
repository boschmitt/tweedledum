/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <list>
#include <tweedledum/networks/gates/gate_kinds.hpp>
#include <vector>
using namespace std;

namespace tweedledum {

uint32_t get_special_part_of_bin(uint32_t num, uint32_t s, uint32_t e,
                                 uint32_t bit_count)
{
	uint32_t res = num;
	uint32_t all_one = pow(2, bit_count) - 1;
	uint32_t mask = all_one >> bit_count - (e - s + 1);

	res = res << (bit_count - 1 - e);

	res = res >> (s + (bit_count - 1 - e));
	res = res & mask;

	return res;
}

void transpose(std::vector<uint32_t>& matrix)
{
	std::vector<uint32_t> res;
	res = matrix;
	uint32_t bit_count = matrix.size();
	for (auto j = 0; j < bit_count; j++) {
		uint32_t idx = bit_count - 1 - j;
		uint32_t num = 0;
		for (auto i = 0; i < bit_count; i++) {
			uint32_t t;
			cout << "res  " << res[i] << endl;

			t = get_special_part_of_bin(res[i], idx, idx, bit_count);

			t = t << (bit_count - 1 - i);

			num = num ^ t;
		}
		if (idx == 2)
			cout << "num  " << num << endl;
		matrix[j] = num;
	}
}

template<class Network>
std::vector<std::pair<uint16_t, uint16_t>>
lwr_cnot_synthesis(std::vector<uint32_t>& matrix, uint32_t n, uint32_t m)
{
	std::vector<std::pair<uint16_t, uint16_t>> gates;
	uint32_t sec_count = ceil(n / m);
	for (auto sec = 1; sec <= sec_count; sec++) {
		// remove duplicate sub-rows in section sec
		std::vector<uint32_t> patt;
		for (auto i = 0; i < pow(2, m); i++) {
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
			uint32_t diag_one = 1;
			uint32_t start = n - 1 - col;

			if (get_special_part_of_bin(matrix[col], start, start, n)
			    == 0)
				diag_one = 0;
			// remove ones in rows below column col
			for (auto row = col + 1; row < n; row++) {
				uint32_t start = n - 1 - col;
				if (get_special_part_of_bin(matrix[row], start,
				                            start, n)
				    == 1) {

					if (diag_one == 0) {

						matrix[col] ^= matrix[row];
						gates.push_back({row, col});
						diag_one = 1;
					}

					matrix[row] ^= matrix[col];

					gates.push_back({col, row});
				}
			}
		}
	}

	return gates;
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

	gates_l = lwr_cnot_synthesis<Network>(matrix, nqubits, partition_size);

	transpose(matrix);
	cout << "matrix size   " << matrix.size() << endl;
	for (auto i = 0; i < matrix.size(); i++) {
		cout << matrix[i] << endl;
	}
	gates_u = lwr_cnot_synthesis<Network>(matrix, nqubits, partition_size);
	// combine lower/upper triangular synthesis
	// swap ctrl/targ of cnot gates in gates_u
	std::list<std::pair<uint16_t, uint16_t>> gates_u_swap;
	for (auto e = 0; e < gates_u.size(); e++) {
		std::pair<uint16_t, uint16_t> temp;
		temp.first = gates_u[e].second;
		temp.second = gates_u[e].first;
		gates_u_swap.push_back(temp);
	}

	std::reverse(gates_l.begin(), gates_l.end());
	for (const auto [c, t] : gates_u_swap) {
		net.add_controlled_gate(gate_kinds_t::cx, c, t);
	}
	for (const auto [c, t] : gates_l) {
		net.add_controlled_gate(gate_kinds_t::cx, c, t);
	}
}

} // namespace tweedledum
