/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_kinds.hpp"

#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace tweedledum {

/*! \brief ...
 */
template<class Network>
void lin_comb_synth_gray(Network& net, std::vector<std::pair<uint32_t, float>> parities,
                         std::vector<uint32_t> const& qubits_map)
{
	const auto nqubits = qubits_map.size();
	std::vector<std::pair<uint16_t, uint16_t>> gates;

	std::vector<uint32_t> parity_gates;
	std::vector<uint32_t> line_parity_val(nqubits);
	for (auto i = 0u; i < nqubits; i++)
		line_parity_val[i] = 1 << i;

	std::vector<uint32_t> grayCodes;
	for (auto i = 0u; i < (1u << nqubits); i++) {
		grayCodes.emplace_back((i >> 1) ^ i);
	}

	for (auto i = nqubits - 1u; i > 0; i--) { // i is target
		for (auto j = (1u << (i + 1)) - 1u; j > (1u << i); j--) {
			uint32_t temp = grayCodes[j] ^ grayCodes[j - 1u];

			gates.emplace_back(log2(temp), i);

			line_parity_val[i] ^= line_parity_val[log2(temp)];
			parity_gates.push_back(line_parity_val[i]);
		}
		uint32_t temp = grayCodes[1 << i] ^ grayCodes[(1u << (i + 1)) - 1u];
		gates.emplace_back(log2(temp), i);
		line_parity_val[i] ^= line_parity_val[log2(temp)];
		parity_gates.push_back(line_parity_val[i]);
	}

	uint32_t idx = 0;
	for (const auto [c, t] : gates) {
		net.add_gate(gate_kinds_t::cx, qubits_map[c], qubits_map[t]);
		for (auto i = 0u; i < parities.size(); i++) {
			if (parity_gates[idx] == parities[i].first) {
				if (parities[i].second != -1) {
					net.add_gate(gate_kinds_t::rotation_z, qubits_map[t],
					             parities[i].second);
					parities[i].second = -1; /* avoiding the insertion of
					                            one phase gate in two places */
				}
			}
		}
		idx++;
	}
}

template<class Network>
void lin_comb_synth_gray(Network& net, uint32_t nqubits,
                         std::vector<std::pair<uint32_t, float>> parities)
{
	for (auto i = 0u; i < nqubits; ++i)
		net.allocate_qubit();

	std::vector<uint8_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);
	lin_comb_synth_gray(net, parities, qubits_map);
}

template<class Network>
void lin_comb_synth_binary(Network& net, std::vector<std::pair<uint32_t, float>> parities,
                           std::vector<uint32_t> const& qubits_map)
{
	const auto nqubits = qubits_map.size();
	std::vector<std::pair<uint16_t, uint16_t>> gates;

	std::vector<uint32_t> parity_gates;
	std::vector<uint32_t> lp_val(nqubits);
	for (auto i = 0u; i < nqubits; i++)
		lp_val[i] = 1u << i; // line parity value

	for (auto i = 1u; i < (1u << nqubits); i++) {
		if ((i ^ (1 << (i - 1))) != 0) {
			uint32_t first_num = floor(log2(i));
			for (auto j = 0u; j < nqubits; j++) {
				if ((first_num != j) && ((lp_val[j] ^ lp_val[first_num]) == i)) {
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
		auto it = std::find_if(parities.begin(), parities.end(),
		                       [&](auto p) { return p.first == 1 << i; });
		if (it == parities.end())
			continue;
		net.add_gate(gate_kinds_t::rotation_z, qubits_map[i], it->second);

		it->second = -1;
	}

	uint32_t idx = 0;
	for (const auto [c, t] : gates) {
		net.add_gate(gate_kinds_t::cx, qubits_map[c], qubits_map[t]);

		for (auto i = 0u; i < parities.size(); i++)
			if (parity_gates[idx] == parities[i].first)
				if (parities[i].second != -1) {
					net.add_gate(gate_kinds_t::rotation_z, qubits_map[t],
					             parities[i].second);
					/* avoiding the insertion of
					 * one phase gate in two places */
					parities[i].second = -1;
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
                           std::vector<std::pair<uint32_t, float>> parities)
{
	for (auto i = 0u; i < nqubits; ++i)
		net.allocate_qubit();

	std::vector<uint32_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);
	lin_comb_synth_binary(net, parities, qubits_map);
}

} /* namespace tweedledum */
