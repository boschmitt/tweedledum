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
 * \param quantum_ntk Reversible quantum circuit
 * \param inputs Qubits which are primary inputs (all other qubits are assumed to be 0)
 * \param outputs Qubits for which a primary output should be created
 */
template<class LogicNtk, class QuantumNtk>
LogicNtk to_logic_network(QuantumNtk const& quantum_ntk)
{
	static_assert(mockturtle::is_network_type_v<LogicNtk>,
	              "Logic network is not a network type");
	static_assert(mockturtle::has_create_pi_v<LogicNtk>,
	              "Logic network does not implement the create_pi method");
	static_assert(mockturtle::has_create_po_v<LogicNtk>,
	              "Logic network does not implement the create_po method");
	static_assert(mockturtle::has_create_nary_and_v<LogicNtk>,
	              "Logic network does not implement the create_nary_and method");
	static_assert(mockturtle::has_create_xor_v<LogicNtk>,
	              "Logic network does not implement the create_xor method");

	using signal_type = typename LogicNtk::signal; 
	// TODO: make sure the network only uses X, CX or MCX gates

	LogicNtk logic_ntk;
	std::vector<signal_type> qubit_to_signal(quantum_ntk.num_qubits(),
	                                         logic_ntk.get_constant(false));

	quantum_ntk.foreach_qubit([&](io_id const qubit) {
		if (quantum_ntk.is_input(qubit)) {
			qubit_to_signal[qubit] = logic_ntk.create_pi();
		}
	});

	quantum_ntk.foreach_gate([&](auto node) {
		std::vector<signal_type> controls;

		node.gate.foreach_control([&](auto control) { 
			controls.push_back(qubit_to_signal[control] ^ control.is_complemented());
		});

		const auto ctrl_signal = logic_ntk.create_nary_and(controls);

		node.gate.foreach_target([&](auto target) {
			qubit_to_signal[target] = logic_ntk.create_xor(qubit_to_signal[target], ctrl_signal);
		});
	});

	uint32_t num_outputs = 0;
	quantum_ntk.foreach_qubit([&](io_id const qubit) {
		if (quantum_ntk.is_output(qubit)) {
			++num_outputs;
		}
	});
	for (uint32_t i = 0; i < num_outputs; ++i) {
		logic_ntk.create_po(qubit_to_signal[quantum_ntk.id(fmt::format("o_{}", i))]);
	}

	return logic_ntk;
}

} // namespace tweedledum
