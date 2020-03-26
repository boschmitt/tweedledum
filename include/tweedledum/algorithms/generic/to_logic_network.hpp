/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire_id.hpp"

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
	using op_type = typename QuantumNtk::op_type;
	// TODO: make sure the network only uses X, CX or MCX gates

	LogicNtk logic_ntk;
	std::vector<signal_type> qubit_to_signal(quantum_ntk.num_qubits(),
	                                         logic_ntk.get_constant(false));

	quantum_ntk.foreach_wire([&](wire_id const wire) {
		if (!wire.is_qubit()) {
			return;
		}
		switch (quantum_ntk.wire_mode(wire)) {
		case wire_modes::in:
		case wire_modes::inout:
			qubit_to_signal[wire] = logic_ntk.create_pi();
			break;
			
		default:
			break;
		}
	});

	quantum_ntk.foreach_op([&](op_type const& op) {
		std::vector<signal_type> controls;

		op.foreach_control([&](wire_id control) { 
			controls.push_back(qubit_to_signal[control] ^ control.is_complemented());
		});

		auto const ctrl_signal = logic_ntk.create_nary_and(controls);

		op.foreach_target([&](wire_id target) {
			qubit_to_signal[target] = logic_ntk.create_xor(qubit_to_signal[target], ctrl_signal);
		});
	});

	uint32_t num_pos = 0;
	quantum_ntk.foreach_wire([&](wire_id const wire) {
		if (!wire.is_qubit()) {
			return;
		}
		switch (quantum_ntk.wire_mode(wire)) {
		case wire_modes::out:
		case wire_modes::inout:
			++num_pos;
			break;

		default:
			break;
		}
	});
	// I need this hack, otherwise the outputs might be permuted
	for (uint32_t po = 0; po < num_pos; ++po) {
		logic_ntk.create_po(qubit_to_signal[quantum_ntk.wire(fmt::format("__o_{}", po))]);
	}
	return logic_ntk;
}

} // namespace tweedledum
