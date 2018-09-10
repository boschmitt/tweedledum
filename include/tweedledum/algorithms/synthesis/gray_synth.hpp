/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"
#include "cnot_patel.hpp"

#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace tweedledum {

struct gray_synth_params {
	bool allow_rewiring{false};
};

namespace detail {
// invert row `adding_row` if `added_row` is 1 in all lists S (both in q and current s)
inline void parities_matrix_update(
    std::vector<std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, uint32_t>>& q,
    std::vector<uint32_t>& s, uint32_t adding_row, uint32_t added_row)
{
	for (auto& stack_elem : q)
		for (auto& S_elem : std::get<0>(stack_elem))
			S_elem ^= ((S_elem >> adding_row) & 1) << added_row;

	for (auto& elem : s)
		elem ^= ((elem >> adding_row) & 1) << added_row;
}

inline uint32_t extract_row_of_vector(std::vector<uint32_t> const& p, uint32_t row)
{
	uint32_t row_val{0};
	for (auto i = 0u; i < p.size(); i++)
		row_val ^= ((p[i] >> row) & 1) << i;
	return row_val;
}

inline std::vector<uint32_t> extract_special_parities(
    std::vector<uint32_t> const& p, uint32_t idx, uint32_t value)
{
	std::vector<uint32_t> out;
	for (const auto i : p)
		if (((i >> idx) & 1) == value)
			out.push_back(i);
	return out;
}

template<typename Container, typename Iterator>
Container permute(Container const& container, Iterator begin, Iterator end)
{
	Container copy{container};
	std::transform(begin, end, copy.begin(),
	               [&](auto i) { return container[i]; });
	return copy;
}

template<class Network>
void add_remainder_network(Network& net, std::vector<uint32_t>& matrix,
                           std::vector<uint8_t> const& qubits_map,
                           bool find_best_perm)
{
	auto const old_size = net.num_gates();
	uint32_t best_gates = std::numeric_limits<uint32_t>::max();
	std::vector<uint32_t> best_permutation(qubits_map.size());
	std::iota(best_permutation.begin(), best_permutation.end(), 0u);
	uint32_t best_partition_size{1};

	/* we mark all nodes added by cnot_patel so that we can remove them
	 * after we computed the improvement. */
	net.default_mark(1);

	auto perm = best_permutation;
	do {
		for (auto p = 1u; p <= qubits_map.size(); ++p) {
			/* copy matrix since cnot_patel modifies it */
			auto matrix_copy
			    = permute(matrix, perm.begin(), perm.end());
			cnot_patel(net, matrix_copy, p, qubits_map);

			/* compute improvement */
			if (const auto req_gates = net.num_gates() - old_size;
			    req_gates < best_gates) {
				best_gates = req_gates;
				best_partition_size = p;
				best_permutation = perm;
			}
			net.remove_marked_nodes();
			assert(net.num_gates() == old_size);
		}
	} while (find_best_perm
	         && std::next_permutation(perm.begin(), perm.end()));
	net.default_mark(0);
	matrix
	    = permute(matrix, best_permutation.begin(), best_permutation.end());
	cnot_patel(net, matrix, best_partition_size, qubits_map);
	//std::cout << "required gates: " << (net.num_gates() - old_size) << "\n";
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
void gray_synth(Network& net, std::vector<uint32_t> parities,
                std::vector<float> Ts, std::vector<uint8_t> const& qubits_map,
                gray_synth_params const& ps = {})
{
	const auto nqubits = qubits_map.size();

	std::vector<std::pair<uint16_t, uint16_t>> gates;
	std::vector<uint32_t> in_lines(nqubits);
	std::iota(in_lines.begin(), in_lines.end(), 0u);

	std::vector<std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, uint32_t>> Q;
	/* -1 is first initialize of epsilon that update after first iteration */
	Q.emplace_back(parities, in_lines, nqubits);

	/* managing phase gates */
	std::vector<uint32_t> parity_gates;
	std::vector<uint32_t> line_parity_val(nqubits);
	for (auto i = 0u; i < nqubits; i++)
		line_parity_val[i] = 1 << i;

	while (!Q.empty()) {
		auto [S, I, ID] = Q.back();
		Q.pop_back(); /* remove top of stack */
		if (S.empty())
			continue;

		if (ID != nqubits)
			for (auto j = 0u; j < nqubits; j++) {
				if (j == ID)
					continue;
				/* xj must exist in all parities of S matrix */
				if (detail::extract_row_of_vector(S, j)
				    != ((1 << S.size()) - 1))
					continue;

				/* insert gate and update parity matrix */
				gates.emplace_back(j, ID);
				line_parity_val[ID] ^= line_parity_val[j];
				parity_gates.push_back(line_parity_val[ID]);
				detail::parities_matrix_update(Q, S, ID, j);
			}

		if (I.empty())
			continue;

		uint32_t max{0u};
		std::vector<uint32_t>::iterator max_it = I.end();

		for (auto it = I.begin(); it != I.end(); ++it) {
			const uint32_t num
			    = detail::extract_row_of_vector(S, *it);
			const uint32_t one_bits = __builtin_popcount(num);
			const uint32_t zero_bits = S.size() - one_bits;

			const auto temp
			    = (one_bits > zero_bits) ? one_bits : zero_bits;
			if (temp > max) {
				max = temp;
				max_it = it;
			}
		}

		const auto S0 = detail::extract_special_parities(S, *max_it, 0);
		const auto S1 = detail::extract_special_parities(S, *max_it, 1);
		const auto max_idx = *max_it;
		I.erase(max_it);

		Q.emplace_back(S1, I, ID == nqubits ? max_idx : ID);
		Q.emplace_back(S0, I, ID);
	} /* end while */

	/* making network */
	/* applying phase gate in the first of line when parity consist of just one variable */
	for (auto i = 0u; i < nqubits; ++i) {
		const auto it
		    = std::find(parities.begin(), parities.end(), 1 << i);
		if (it == parities.end())
			continue;
		const auto idx = std::distance(parities.begin(), it);
		net.add_z_rotation(qubits_map[i], Ts[idx]);
		Ts[idx] = -1;
	}

	uint32_t idx = 0;
	for (const auto [c, t] : gates) {
		net.add_controlled_gate(gate_kinds_t::cx, qubits_map[c],
		                        qubits_map[t]);
		for (auto i = 0u; i < Ts.size(); i++) {
			if (parity_gates[idx] == parities[i]) {
				if (Ts[i] != -1) {
					net.add_z_rotation(qubits_map[t], Ts[i]);
					Ts[i] = -1; /* avoiding the insertion of
					               one phase gate in two places */
				}
			}
		}
		idx++;
	}

	/* computing remaining matrix */
	std::vector<uint32_t> matrix(nqubits, 0);
	for (auto row = 0u; row < nqubits; ++row) {
		matrix[row] = 1 << row;
	}
	for (auto it = gates.rbegin(); it != gates.rend(); ++it) {
		const auto [c, t] = *it;
		matrix[t] ^= matrix[c];
	}

	/* add remainder network */
	detail::add_remainder_network<Network>(net, matrix, qubits_map,
	                                       ps.allow_rewiring);
}

template<class Network>
void gray_synth(Network& net, uint32_t nqubits, std::vector<uint32_t> parities,
                std::vector<float> Ts, gray_synth_params const& ps = {})
{
	for (auto i = 0u; i < nqubits; ++i)
		net.allocate_qubit();

	std::vector<uint8_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);
	gray_synth(net, parities, Ts, qubits_map, ps);
}

} /* namespace tweedledum */
