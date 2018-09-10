/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"

#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace tweedledum {

/*! \brief ...
 */
template<class Network>
void lin_comb_synth_gray(Network& net, std::vector<uint32_t> parities,
                         std::vector<float> Ts,
                         std::vector<uint8_t> const& qubits_map)
{
	const auto nqubits = qubits_map.size();
	std::vector<std::pair<uint16_t, uint16_t>> gates;

	std::vector<uint32_t> parity_gates;
	std::vector<uint32_t> line_parity_val(nqubits);
	for (auto i = 0u; i < nqubits; i++)
		line_parity_val[i] = 1 << i;

	std::vector<uint32_t> grayCodes;
	for (auto i = 0u; i < (1 << nqubits); i++) {
		grayCodes.emplace_back((i >> 1) ^ i);
	}

	for (auto i = nqubits - 1; i > 0; i--) { // i is target
		for (auto j = (1 << (i + 1)) - 1; j > (1 << i); j--) {
			uint32_t temp = grayCodes[j] ^ grayCodes[j - 1];

			gates.emplace_back(log2(temp), i);

			line_parity_val[i] ^= line_parity_val[log2(temp)];
			parity_gates.push_back(line_parity_val[i]);
		}
		uint32_t temp = grayCodes[1 << i] ^ grayCodes[(1 << (i + 1)) - 1];
		gates.emplace_back(log2(temp), i);
		line_parity_val[i] ^= line_parity_val[log2(temp)];
		parity_gates.push_back(line_parity_val[i]);
	}

	uint32_t idx = 0;
	for (const auto [c, t] : gates) {
		net.add_controlled_gate(gate_kinds_t::cx, qubits_map[c],
		                        qubits_map[t]);
		for (auto i = 0u; i < Ts.size(); i++) {
			if (parity_gates[idx] == parities[i]) {
				if (Ts[i] != -1) {
					net.add_z_rotation(qubits_map[t], Ts[i]);
					Ts[i]
					    = -1; /* avoiding the insertion of
					                      one phase gate in two places */
				}
			}
		}
		idx++;
	}
}

template<class Network>
void lin_comb_synth_gray(Network& net, uint32_t nqubits,
                         std::vector<uint32_t> parities, std::vector<float> Ts)
{
	for (auto i = 0u; i < nqubits; ++i)
		net.allocate_qubit();

	std::vector<uint8_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);
	lin_comb_synth_gray(net, parities, Ts, qubits_map);
}

template<class Network>
void lin_comb_synth_binary(Network& net, std::vector<uint32_t> parities,
                           std::vector<float> Ts,
                           std::vector<uint8_t> const& qubits_map)
{
	const auto nqubits = qubits_map.size();
	std::vector<std::pair<uint16_t, uint16_t>> gates;

	std::vector<uint32_t> parity_gates;
	std::vector<uint32_t> lp_val(nqubits);
	for (auto i = 0u; i < nqubits; i++)
		lp_val[i] = 1 << i; // line parity value

	for (auto i = 1u; i < (1 << nqubits); i++) {
		if ((i ^ (1 << (i - 1))) != 0) {
			uint32_t first_num = floor(log2(i));
			for (auto j = 0u; j < nqubits; j++) {
				if ((first_num != j)
				    && ((lp_val[j] ^ lp_val[first_num]) == i)) {
					lp_val[first_num] ^= lp_val[j];
					gates.emplace_back(j, first_num);
					parity_gates.push_back(lp_val[first_num]);
				}
			}
		}
	}
	// return lines
	for (int i = nqubits - 1; i > 0; i--)
		gates.emplace_back(i - 1, i);

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

		for (auto i = 0u; i < Ts.size(); i++)
			if (parity_gates[idx] == parities[i])
				if (Ts[i] != -1) {
					net.add_z_rotation(qubits_map[t], Ts[i]);
					/* avoiding the insertion of
					 * one phase gate in two places */
					Ts[i] = -1;
				}
		idx++;
	}

	// extract partitioning
	std::vector<uint32_t> t_lines(nqubits, 0u);
	for (const auto [c, t] : gates)
		if (t != c)
			t_lines[t] += 1;
	std::sort(t_lines.begin(), t_lines.end());
}

template<class Network>
void lin_comb_synth_binary(Network& net, uint32_t nqubits,
                           std::vector<uint32_t> parities, std::vector<float> Ts)
{
	for (auto i = 0u; i < nqubits; ++i)
		net.allocate_qubit();

	std::vector<uint8_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);
	lin_comb_synth_binary(net, parities, Ts, qubits_map);
}

} /* namespace tweedledum */
