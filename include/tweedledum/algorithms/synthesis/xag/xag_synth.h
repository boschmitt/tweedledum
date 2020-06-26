/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../ir/Circuit.h"
#include "../../../ir/GateLib.h"
#include "../../../ir/Wire.h"
#include "CollapsedXAG.h"

#include <array>
#include <cassert>
#include <mockturtle/networks/xag.hpp>
#include <vector>

namespace tweedledum {
#pragma region Implementation details
namespace xag_synth_detail {

enum class Action : uint8_t {
	compute,
	cleanup,
};

struct Step {
	Action action;
	uint32_t node;
	Step(Action a, uint32_t n) : action(a), node(n) {}
};

void pre_assign_qubits(CollapsedXAG& collapsed_xag,
    std::vector<WireRef> const& qubits, std::vector<WireRef>& to_qubit)
{
	uint32_t qubit_idx = 0;
	// Assing qubits to inputs
	for (; qubit_idx < collapsed_xag.num_inputs; ++qubit_idx) {
		to_qubit[qubit_idx + 1] = qubits.at(qubit_idx);
	}
	// Assing qubits to outputs and Gate gates that are only referenced
	// once and by a output.
	for (Output& output : collapsed_xag.outputs) {
		for (uint32_t id : output.fanin) {
			Gate& gate = collapsed_xag.gates[id];
			if (!gate.is_and || gate.ref_count > 1) {
				continue;
			}
			to_qubit[id] = qubits[qubit_idx];
			gate.cleanup = 0;
			output.cleanup = 1;
			break;
		}
		qubit_idx += 1;
	}
}

void post_assign_qubits(CollapsedXAG& collapsed_xag,
    std::vector<WireRef> const& qubits,
    std::vector<WireRef>& to_qubit)
{
	// Assing qubits to outputs that haven't been assigned and that have 
	// only one Gate gate as input
	uint32_t qubit_idx = collapsed_xag.num_inputs;
	for (Output& output : collapsed_xag.outputs) {
		if (output.cleanup || output.num_ands != 1) {
			qubit_idx += 1;
			continue;
		}
		for (uint32_t id : output.fanin) {
			Gate& gate = collapsed_xag.gates[id];
			assert(gate.ref_count);
			if (!gate.is_and || to_qubit[id] != WireRef::invalid()) {
				continue;
			}
			to_qubit[id] = qubits[qubit_idx];
			gate.cleanup = 0;
			output.cleanup = 2;
			break;
		}
		qubit_idx += 1;
	}
	qubit_idx = collapsed_xag.num_inputs;
	for (Output& output : collapsed_xag.outputs) {
		if (output.cleanup) {
			qubit_idx += 1;
			continue;
		}
		for (uint32_t id : output.fanin) {
			Gate& gate = collapsed_xag.gates[id];
			if (!gate.is_and || to_qubit[id] != WireRef::invalid()) {
				continue;
			}
			if (gate.ref_count == 1) {
				to_qubit[id] = qubits[qubit_idx];
				gate.cleanup = 0;
				break;
			}
		}
		qubit_idx += 1;
	}
}

void try_cleanup(CollapsedXAG& collapsed_xag,
    std::vector<uint32_t> const& gates, std::vector<Step>& steps)
{
	for (uint32_t i_index : gates) {
		Gate& fanin = collapsed_xag.gates[i_index];
		fanin.ref_count -= 1;
		if (fanin.ref_count == 0 && fanin.is_and) {
			steps.emplace_back(Action::cleanup, i_index);
			try_cleanup(collapsed_xag, fanin.in0, steps);
			try_cleanup(collapsed_xag, fanin.in1, steps);
			try_cleanup(collapsed_xag, fanin.in01, steps);
		}
	}
}

void add_parity(Circuit& circuit, std::vector<WireRef> const& qubits)
{
	if (qubits.size() == 1) {
		return;
	}
	circuit.create_instruction(GateLib::Parity(), qubits);
}

void add_gate(Circuit& circuit, Gate const& gate,
    std::vector<WireRef> const& to_qubit, WireRef target)
{
	std::vector<WireRef> in0;
	std::vector<WireRef> in1;
	std::vector<WireRef> in01;
	for (uint32_t i : gate.in0) {
		in0.push_back(to_qubit.at(i));
	}
	for (uint32_t i : gate.in1) {
		in1.push_back(to_qubit.at(i));
	}
	for (uint32_t i : gate.in01) {
		in01.push_back(to_qubit.at(i));
	}
	// Compute the inputs to the Toffoli gate (inplace)
	add_parity(circuit, in0);
	if (!in01.empty()) {
		add_parity(circuit, in01);
		in1.push_back(in01.back());
		circuit.create_instruction(
		    GateLib::X(), {in01.back()}, in0.back());
	}
	add_parity(circuit, in1);
	// Compute Toffoli
	WireRef c0 = gate.is_negated[0] ? !in0.back() : in0.back();
	WireRef c1 = gate.is_negated[1] ? !in1.back() : in1.back();
	circuit.create_instruction(GateLib::X(), {c0, c1}, target);
	// Cleanup the input to the Toffoli gate
	add_parity(circuit, in1);
	if (!in01.empty()) {
		circuit.create_instruction(
		    GateLib::X(), {in01.back()}, in0.back());
		add_parity(circuit, in01);
	}
	add_parity(circuit, in0);
}

void execute_steps(std::vector<Step> const& steps,
    std::vector<Gate> const& gates, std::vector<WireRef>& to_qubit,
    Circuit& circuit)
{
	for (Step const& step : steps) {
		Gate const& gate = gates[step.node];
		switch (step.action) {
		case Action::compute:
			if (to_qubit[step.node] == WireRef::invalid()) {
				to_qubit[step.node] = circuit.request_ancilla();
			}
			add_gate(circuit, gate, to_qubit, to_qubit[step.node]);
			break;

		case Action::cleanup:
			circuit.release_ancilla(to_qubit[step.node]);
			add_gate(circuit, gate, to_qubit, to_qubit[step.node]);
			break;
		}
	}
}

void compute_outputs(CollapsedXAG& collapsed_xag, std::vector<WireRef> const& qubits, std::vector<WireRef> const& to_qubit,
    Circuit& circuit)
{
	uint32_t qubit_idx = collapsed_xag.num_inputs;
	for (Output& output : collapsed_xag.outputs) {
		if (output.cleanup == 2) {
			qubit_idx += 1;
			continue;
		}
		std::vector<WireRef> qs;
		for (uint32_t id : output.fanin) {
			if (to_qubit[id] == qubits[qubit_idx]) {
				continue;
			}
			qs.push_back(to_qubit[id]);
		}
		qs.push_back(qubits[qubit_idx]);
		add_parity(circuit, qs);
		qubit_idx += 1;
	}

	qubit_idx = collapsed_xag.num_inputs;
	for (Output& output : collapsed_xag.outputs) {
		if (output.cleanup != 2) {
			qubit_idx += 1;
			continue;
		}
		// assert(gates[i].cleanup == 2);
		std::vector<WireRef> qs;
		for (uint32_t id : output.fanin) {
			if (to_qubit[id] == qubits[qubit_idx]) {
				continue;
			}
			qs.push_back(to_qubit[id]);
		}
		qs.push_back(qubits[qubit_idx]);
		add_parity(circuit, qs);
		qubit_idx += 1;
	}

	qubit_idx = collapsed_xag.num_inputs;
	for (Output& output : collapsed_xag.outputs) {
		if (output.is_negated) {
			circuit.create_instruction(GateLib::X(), {qubits[qubit_idx]});
		}
		qubit_idx += 1;
	}
}

void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits,
    mockturtle::xag_network const& xag)
{
	CollapsedXAG collapsed_xag = collapse_xag(xag);
	std::vector<WireRef> to_qubit(collapsed_xag.size(), WireRef::invalid());

	pre_assign_qubits(collapsed_xag, qubits, to_qubit);

	// Compute steps 
	std::vector<Step> steps;
	uint32_t index = 0;
	for (Gate& gate : collapsed_xag.gates) {
		if (!gate.is_and) {
			index += 1;
			continue;
		}
		steps.emplace_back(Action::compute, index);
		// Eagerly try to cleanup
		if (gate.cleanup == 2) {
			try_cleanup(collapsed_xag, gate.in0, steps);
			try_cleanup(collapsed_xag, gate.in1, steps);
			try_cleanup(collapsed_xag, gate.in01, steps);
		}
		index += 1;
	}

	post_assign_qubits(collapsed_xag, qubits, to_qubit);
	execute_steps(steps, collapsed_xag.gates, to_qubit, circuit);
	compute_outputs(collapsed_xag, qubits, to_qubit, circuit);
	
	// Clean up whats is left
	std::for_each(steps.crbegin(), steps.crend(), [&](Step const& step) {
		Gate& gate = collapsed_xag.gates[step.node];
		if (gate.cleanup == 0) {
			return;
		}
		circuit.release_ancilla(to_qubit[step.node]);
		add_gate(circuit, gate, to_qubit, to_qubit[step.node]);
	});
}

} // namespace xag_synth_detail
#pragma endregion

inline void xag_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    mockturtle::xag_network const& xag)
{
	xag_synth_detail::synthesize(circuit, qubits, xag);
}

inline Circuit xag_synth(mockturtle::xag_network const& xag)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");
	// Create the necessary qubits
	uint32_t const nun_qubits = xag.num_pis() + xag.num_pos();
	std::vector<WireRef> wires;
	wires.reserve(nun_qubits);
	for (uint32_t i = 0u; i < nun_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	xag_synth(circuit, wires, xag);
	return circuit;
}

} // namespace tweedledum