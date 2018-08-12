/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace tweedledum {

namespace detail {
void parities_matrix_update(
    std::vector<std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, uint32_t>>& q,
    std::vector<uint32_t>& s, uint32_t adding_row, uint32_t added_row)
{
	uint32_t t1, t2;
	for (auto i = 0; i < q.size(); i++) {

		for (auto j = 0u; j < (std::get<0>(q[i]).size()); j++) {
			t1 = (std::get<0>(q[i])[j] >> adding_row) & 1;
			t2 = (std::get<0>(q[i])[j] >> added_row) & 1;
			if (t1) {
				std::get<0>(q[i])[j] ^= (1 << added_row);
			}
		}
	}
	for (auto i = 0; i < s.size(); i++) {
		t1 = (s[i] >> adding_row) & 1;
		t2 = (s[i] >> added_row) & 1;
		if (t1) {
			s[i] ^= (1 << added_row);
		}
	}
}

uint32_t extract_one_bits(uint32_t number)
{
	uint32_t temp;
	uint32_t one_bits = 0;
	while (number != 0) {
		temp = number & 1; // first bit of number
		number >>= 1;
		if (temp)
			one_bits += 1;
	}

	return one_bits;
}

uint32_t extract_row_of_vector(std::vector<uint32_t> p, uint32_t row)
{
	uint32_t col_count = p.size();
	uint32_t row_val = 0;
	uint32_t temp;
	for (auto i = 0u; i < col_count; i++) {
		temp = (p[i] >> row) & 1;
		row_val ^= (temp << (col_count - 1 - i));
	}
	return row_val;
}

std::vector<uint32_t> extract_special_parities(std::vector<uint32_t> p,
                                               uint32_t idx, uint32_t value)
{
	std::vector<uint32_t> out;
	uint32_t temp;
	for (auto i = 0u; i < p.size(); i++) {
		temp = (p[i] >> idx) & 1;
		if (temp == value)
			out.push_back(p[i]);
	}
	return out;
}

std::vector<uint32_t> creating_parity(std::vector<uint32_t> v1,
                                      std::vector<uint32_t> v2)
{
	std::vector<uint32_t> out;
	out = v1;
	out.insert(std::end(out), std::begin(v2), std::end(v2));
	std::sort(out.begin(), out.end());
	for (auto it = out.begin(); (it + 1) <= out.end();) {
		if (*it == *(it + 1)) {
			out.erase(it);
			out.erase(it);
		} else
			++it;
	}
	return out;
}

std::vector<uint32_t> extract_one_bit_nums(uint32_t value)
{
	std::vector<uint32_t> out;
	uint32_t idx = 0, temp;
	while (value != 0) {
		temp = value & 1; // first bit of number
		value >>= 1;
		if (temp)
			out.push_back(idx);
		idx++;
	}
	return out;
}

std::vector<std::vector<uint32_t>> creating_parity_of_T(std::vector<uint32_t> p)
{
	std::vector<std::vector<uint32_t>> out;
	std::vector<uint32_t> temp;
	for (auto i = 0u; i < p.size(); i++) {
		temp = extract_one_bit_nums(p[i]);
		out.push_back(temp);
	}
	return out;
}



} // namespace detail

/*! \brief Gray synthesis for CNOT-PHASE networks.
 *
 * This algorithm is based on the work in [M. Amy, P. Azimzadeh, M. Mosca: On
 * the complexity of CNOT-PHASE circuits, in: arXiv:1712.01859, 2017.]
 *
 * The following code shows how to apply the algorithm to the example in the
 * original paper.
 *
   \verbatim embed:rst

   .. code-block:: c++

      dag_path<qc_gate> network = ...;

      float T{0.39269908169872414}; // PI/8
      std::vector<std::pair<uint32_t, float>> parities{
          {{0b0110, T},
           {0b0001, T},
           {0b1001, T},
           {0b0111, T},
           {0b1011, T},
           {0b0011, T}}
      };
      gray_synth(network, 4, parities);
   \endverbatim
 */
template<class Network>
void gray_synth(Network& net, uint32_t nqubits, std::vector<uint32_t> parities,
                std::vector<float> Ts)
{
	std::vector<std::pair<uint16_t, uint16_t>> gates;
	std::vector<uint32_t> in_lines;
	for (auto i = 0u; i < nqubits; i++) {
		in_lines.push_back(i);
	}
	std::vector<std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, uint32_t>> Q;
	Q.push_back(
	    {parities, in_lines, nqubits}); // -1 is first initialize of epsilon
	                                    // that update after first iteration

	std::vector<std::vector<uint32_t>> corr_parities_of_Ts
	    = detail::creating_parity_of_T(parities);
	std::vector<std::vector<uint32_t>> parity_gates;
	std::vector<std::vector<uint32_t>> line_parity_val;
	for (auto i = 0u; i < nqubits; i++) {
		line_parity_val.push_back({i});
	}

	while (!Q.empty()) {

		std::vector<uint32_t> S = std::get<0>(Q.back());
		std::vector<uint32_t> I = std::get<1>(Q.back());
		uint32_t ID = std::get<2>(Q.back());
		Q.pop_back();  // remove top of stack
		if (S.empty()) // | I.empty())
			continue;

		else if (ID != nqubits) {
			for (auto j = 0u; j < nqubits; j++) {
				if (j == ID)
					continue;
				if (detail::extract_row_of_vector(S, j)
				    == ((1 << S.size())
				        - 1)) { // xj input exist in all parities of S matrix
					gates.push_back({j, ID});
					std::vector<uint32_t> temp
					    = detail::creating_parity(
					        line_parity_val[j],
					        line_parity_val[ID]);
					line_parity_val.at(ID) = temp;
					parity_gates.push_back(temp);

					detail::parities_matrix_update(Q, S, ID,
					                               j);
				}
			}
		}
		if (!I.empty()) {
			uint32_t max = 0;
			uint32_t max_idx = nqubits;
			uint32_t one_bits, zero_bits, num, temp;

			for (auto i = 0u; i < I.size(); i++) {
				num = detail::extract_row_of_vector(S, I[i]);

				one_bits = detail::extract_one_bits(num);
				zero_bits = S.size() - one_bits;

				temp = (one_bits > zero_bits) ? one_bits :
				                                zero_bits;
				if (temp > max) {
					max = temp;
					max_idx = I[i];
				}
			}

			std::vector<uint32_t> S0;
			std::vector<uint32_t> S1;

			S0 = detail::extract_special_parities(S, max_idx, 0);
			S1 = detail::extract_special_parities(S, max_idx, 1);
			I.erase(std::remove(I.begin(), I.end(), max_idx),
			        I.end());

			if (ID == nqubits)
				Q.push_back({S1, I, max_idx});
			else
				Q.push_back({S1, I, ID});
			Q.push_back({S0, I, ID});
			// for (auto ii = 0; ii < S.size(); ii++)
			// 	std::cout << S[ii] << std::endl;

		} // end if I.empty()
		
	} // end while

	std::cout << "gates count:     " << gates.size() << std::endl;
	for (auto ii = 0; ii < gates.size(); ii++)
		std::cout <<"CNOT   "<< std::get<0>(gates[ii]) << "    "
		          << std::get<1>(gates[ii]) << std::endl;

	for (auto i = 0u; i < corr_parities_of_Ts.size(); i++) {
		for (auto j = 0u; j < corr_parities_of_Ts[i].size(); j++) {
			std::cout << corr_parities_of_Ts[i][j] << "   ";
		}
		std::cout << "\n";
	}

	// applying Phase gates

	// for (const auto [c, t] : gates) {
	// 	net.add_controlled_gate(gate_kinds_t::cx, c, t);
	// }

} // end function

} // namespace tweedledum
