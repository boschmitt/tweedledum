/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/netlist.hpp"

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <vector>

namespace tweedledum {

/*! \brief TODO

 * \param
 * \return
 *
 * \algtype synthesis
 * \algexpects
 * \algreturns
 */
template<typename Network>
void esop_phase_synth(Network& network, std::vector<qubit_id> const& qubits,
                      kitty::dynamic_truth_table const& function)
{
	for (const auto& cube : kitty::esop_from_pprm(function)) {
		std::vector<qubit_id> controls;
		std::vector<qubit_id> targets;
		for (auto i = 0; i < function.num_vars(); ++i) {
			if (!cube.get_mask(i)) {
				continue;
			}
			assert(cube.get_bit(i));
			if (targets.empty()) {
				targets.emplace_back(qubits[i]);
			} else {
				controls.emplace_back(qubits[i]);
			}
		}
		if (!targets.empty()) {
			network.add_gate(gate::mcz, controls, targets);
		}
	}
}

/*! \brief TODO

 * \param
 * \return
 *
 * \algtype synthesis
 * \algexpects
 * \algreturns
 */
template<class Network>
Network esop_phase_synth(kitty::dynamic_truth_table const& function)
{
	Network network;
	const uint32_t num_qubits = function.num_vars();
	for (auto i = 0u; i < num_qubits; ++i) {
		network.add_qubit();
	}
	std::vector<qubit_id> qubits(num_qubits);
	std::iota(qubits.begin(), qubits.end(), 0u);
	esop_phase_synth(network, qubits, function);
	return network;
}

}; // namespace tweedledum
