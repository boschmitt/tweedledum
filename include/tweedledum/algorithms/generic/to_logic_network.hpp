/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/gates/gate_lib.hpp"
#include "tweedledum/networks/io_id.hpp"

#include <mockturtle/traits.hpp>
#include <vector>

namespace tweedledum {

/*! \brief Convert reversible quantum circuit into logic network.
 *
 * This function creates a logic network from a reversible circuit.  If the
 * quantum circuit contains a non-classical gate, it will return `std::nullopt`,
 * otherwise an optional value that contains a logic network.
 *
 * \param q_network Reversible quantum circuit
 * \param inputs Qubits which are primary inputs (all other qubits are assumed to be 0)
 * \param outputs Qubits for which a primary output should be created
 */
template<class LogicNetwork, class QuantumCircuit>
LogicNetwork to_logic_network(QuantumCircuit const& q_network, std::vector<tweedledum::io_id> const& inputs,
                              std::vector<tweedledum::io_id> const& outputs)
{
	static_assert(mockturtle::is_network_type_v<LogicNetwork>,
	              "Logic network is not a network type");
	static_assert(mockturtle::has_create_pi_v<LogicNetwork>,
	              "Logic network does not implement the create_pi method");
	static_assert(mockturtle::has_create_po_v<LogicNetwork>,
	              "Logic network does not implement the create_po method");
	static_assert(mockturtle::has_create_nary_and_v<LogicNetwork>,
	              "Logic network does not implement the create_nary_and method");
	static_assert(mockturtle::has_create_xor_v<LogicNetwork>,
	              "Logic network does not implement the create_xor method");

	LogicNetwork c_network;
	// TODO: make sure the network only uses X, CX or MCX gates

	std::vector<mockturtle::signal<LogicNetwork>> qubit_to_signal(q_network.num_qubits(),
	                                                              c_network.get_constant(false));

	for (auto qubit : inputs) {
		qubit_to_signal[qubit] = c_network.create_pi();
	}

	q_network.foreach_gate([&](auto node) {
		std::vector<mockturtle::signal<LogicNetwork>> controls;

		node.gate.foreach_control([&](auto control) { 
			controls.push_back(qubit_to_signal[control] ^ control.is_complemented());
		});

		const auto ctrl_signal = c_network.create_nary_and(controls);

		node.gate.foreach_target([&](auto target) {
			qubit_to_signal[target] = c_network.create_xor(qubit_to_signal[target], ctrl_signal);
		});
	});

	for (auto qubit : outputs) {
		c_network.create_po(qubit_to_signal[qubit]);
	}

	return c_network;
}

} // namespace tweedledum
