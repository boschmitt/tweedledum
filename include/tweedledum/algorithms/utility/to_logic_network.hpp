/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire.hpp"

#include <mockturtle/traits.hpp>
#include <vector>

namespace tweedledum {

/*! \brief Converts a reversible quantum circuit into logic network.
 *
 * This function creates a logic network from a reversible circuit.  If the quantum circuit contains
 * a non-classical gate, it will return empty circuit, otherwise an optional value that contains a
 * logic network.
 *
 * \tparam Circuit the __quantum__ circuit type.
 * \tparam LogicNtk the __classical__ network type (from ``mockturtle``).
 * \param[in] circuit the original __quantum__ circuit (__will not be modified__).
 * \returns a mockturtle's logic network equivalent to the circuit.
 */
template<class LogicNtk, class Circuit>
LogicNtk to_logic_network(Circuit const& circuit)
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
	using op_type = typename Circuit::op_type;
	// TODO: make sure the network only uses X, CX or MCX gates

	LogicNtk logic_ntk;
	std::vector<signal_type> qubit_to_signal(circuit.num_qubits(),
	                                         logic_ntk.get_constant(false));

	circuit.foreach_wire([&](wire::id const wire) {
		if (!wire.is_qubit()) {
			return;
		}
		switch (circuit.wire_mode(wire)) {
		case wire::modes::in:
		case wire::modes::inout:
			qubit_to_signal[wire] = logic_ntk.create_pi();
			break;

		default:
			break;
		}
	});

	circuit.foreach_op([&](op_type const& op) {
		std::vector<signal_type> controls;

		op.foreach_control([&](wire::id control) {
			controls.push_back(qubit_to_signal[control] ^ control.is_complemented());
		});

		auto const ctrl_signal = logic_ntk.create_nary_and(controls);

		op.foreach_target([&](wire::id target) {
			qubit_to_signal[target] = logic_ntk.create_xor(qubit_to_signal[target],
			                                               ctrl_signal);
		});
	});

	uint32_t num_pos = 0;
	circuit.foreach_wire([&](wire::id const wire) {
		if (!wire.is_qubit()) {
			return;
		}
		switch (circuit.wire_mode(wire)) {
		case wire::modes::out:
		case wire::modes::inout:
			++num_pos;
			break;

		default:
			break;
		}
	});
	// I need this hack, otherwise the outputs might be permuted
	for (uint32_t po = 0; po < num_pos; ++po) {
		logic_ntk.create_po(qubit_to_signal[circuit.wire(fmt::format("__o_{}", po))]);
	}
	return logic_ntk;
}

} // namespace tweedledum
