/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_kinds.hpp"
#include "../generic/rewrite.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace tweedledum {

struct nct_mapping_params {
	uint32_t controls_threshold = 2u;
};

namespace detail {

// Barenco, A., Bennett, C.H., Cleve, R., DiVincenzo, D.P., Margolus, N., Shor, P., Sleator, T., Smolin,
// J.A. and Weinfurter, H., 1995. Elementary gates for quantum computation. Physical review A, 52(5), p.3457.
template<class Network>
void tofolli_barrenco_decomposition(Network& network, std::vector<uint32_t> const& controls,
                                    uint32_t target, nct_mapping_params const& params)
{
	const auto num_controls = controls.size();
	assert(num_controls >= 2);

	if (num_controls <= params.controls_threshold) {
		network.add_gate(gate_kinds_t::mcx, {target}, controls);
		return;
	}

	std::vector<uint32_t> workspace;
	for (auto i = 0ull; i < network.num_qubits(); ++i) {
		if (i != target && std::find(controls.begin(), controls.end(), i) == controls.end()) {
			workspace.push_back(i);
		}
	}
	const auto workspace_size = workspace.size();
	if (workspace_size == 0) {
		std::cout << "[e] no sufficient helper line found for mapping, break\n";
		return;
	}

	// Check if there are enough empty lines lines
	// Lemma 7.2: If n ≥ 5 and m ∈ {3, ..., ⌈n/2⌉} then a gate can be simulated by a network
	// consisting of 4(m − 2) gates.
	// n is the number of qubits
	// m is the number of controls
	if (network.num_qubits() + 1 >= (num_controls << 1)) {
		workspace.push_back(target);

		// When offset is equal to 0 this is computing the toffoli
		// When offset is 1 this is cleaning up the workspace, that is, restoring the state
		// to their initial state
		for (int offset = 0; offset <= 1; ++offset) {
			for (int i = offset; i < static_cast<int>(num_controls) - 2; ++i) {
				network.add_gate(gate_kinds_t::mcx,
				                 std::vector({controls[num_controls - 1 - i],
				                              workspace[workspace_size - 1 - i]}),
				                 std::vector({workspace[workspace_size - i]}));
			}

			network.add_gate(gate_kinds_t::mcx, std::vector({controls[0], controls[1]}),
			                 std::vector(
			                     {workspace[workspace_size - (num_controls - 2)]}));

			for (int i = num_controls - 2 - 1; i >= offset; --i) {
				network.add_gate(
				    gate_kinds_t::mcx,
				    std::vector<uint32_t>({controls[num_controls - 1 - i],
				                           workspace[workspace_size - 1 - i]}),
				    std::vector<uint32_t>({workspace[workspace_size - i]}));
			}
		}
		return;
	}

	// Not enough qubits in the workspace, extra decomposition step
	// Lemma 7.3: For any n ≥ 5, and m ∈ {2, ... , n − 3} a (n−2)-toffoli gate can be simulated
	// by a network consisting of two m-toffoli gates and two (n−m−1)-toffoli gates
	std::vector<uint32_t> controls0;
	std::vector<uint32_t> controls1;
	for (auto i = 0u; i < (num_controls >> 1); ++i) {
		controls0.push_back(controls[i]);
	}
	for (auto i = (num_controls >> 1); i < num_controls; ++i) {
		controls1.push_back(controls[i]);
	}
	controls1.push_back(workspace_size);
	tofolli_barrenco_decomposition(network, controls0, workspace_size, params);
	tofolli_barrenco_decomposition(network, controls1, target, params);
	tofolli_barrenco_decomposition(network, controls0, workspace_size, params);
	tofolli_barrenco_decomposition(network, controls1, target, params);
}

} /* namespace detail */

/*! \brief Decoposes a circuit with multiple controlled toffoli gates (more than to controls) into a
 * circuit wiht only Not, Cnot, Toffolli (NCT).
 *
 * **Required gate functions:**
 *
 * **Required network functions:**
 */
template<typename Network>
Network nct_mapping(Network const& src, nct_mapping_params const& params = {})
{
	auto gate_rewriter = [&](auto& dest, auto const& gate) {
		if (gate.is(gate_kinds_t::mcx)) {
			switch (gate.num_controls()) {
			case 0:
				gate.foreach_target([&](auto target) {
					dest.add_gate(gate_kinds_t::pauli_x, target);
				});
				break;

			case 1:
				gate.foreach_control([&](auto control) {
					gate.foreach_target([&](auto target) {
						dest.add_gate(gate_kinds_t::cx, control, target);
					});
				});
				break;

			default:
				std::vector<uint32_t> controls;
				std::vector<uint32_t> targets;
				gate.foreach_control(
				    [&](auto control) { controls.push_back(control); });
				gate.foreach_target([&](auto target) { targets.push_back(target); });
				for (auto i = 1ull; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, targets[0], targets[i]);
				}
				detail::tofolli_barrenco_decomposition(dest, controls, targets[0],
				                                       params);
				for (auto i = 1ull; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, targets[0], targets[i]);
				}
				break;
			}
			return true;
		}
		return false;
	};

	auto num_ancillae = 0u;
	src.foreach_node([&](auto const& n) {
		if (n.gate.is(gate_kinds_t::mcx) && n.gate.num_controls() > 2
		    && n.gate.num_controls() + 1 == src.num_qubits()) {
			num_ancillae = 1u;
			return false;
		}
		return true;
	});

	Network dest;
	rewrite_network(dest, src, gate_rewriter, num_ancillae);
	return dest;
}

} // namespace tweedledum
