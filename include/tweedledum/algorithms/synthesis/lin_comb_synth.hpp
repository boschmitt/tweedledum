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
void lin_comb_synth(Network& net, std::vector<uint32_t> parities,
                    std::vector<float> Ts, std::vector<uint8_t> const& qubits_map)
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
void lin_comb_synth(Network& net, uint32_t nqubits,
                    std::vector<uint32_t> parities, std::vector<float> Ts)
{
	for (auto i = 0u; i < nqubits; ++i)
		net.allocate_qubit();

	std::vector<uint8_t> qubits_map(nqubits);
	std::iota(qubits_map.begin(), qubits_map.end(), 0u);
	lin_comb_synth(net, parities, Ts, qubits_map);
}

} /* namespace tweedledum */
