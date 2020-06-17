/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"

#include <cassert>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <vector>

namespace tweedledum {
#pragma region Implementation details
namespace xag_synth_detail {

using XAG = mockturtle::xag_network;
using XAGNode = typename mockturtle::xag_network::node;
using XAGSignal = typename mockturtle::xag_network::signal;
// LTFI = Linear Transitive Fanin
using LTFI = std::vector<XAGSignal>;

struct ParityAnd {
	std::vector<WireRef> in0;
	std::vector<WireRef> in1;
	std::vector<WireRef> in01;
	WireRef target;
	bool cleanup;
	std::array<bool, 2> is_complemented;

	ParityAnd() : target(WireRef::invalid()), cleanup(false), is_complemented({{false, false}}) {}
};

struct State {
	XAG const& xag;
	mockturtle::node_map<WireRef, XAG> to_qubit; // XAG node -> qubit
	mockturtle::node_map<LTFI, XAG> ltfi; // XAG node -> ltfi

	// std::vector<WireRef> const& qubits;
	Circuit& circuit;

	State(XAG const& xag, Circuit& circuit)
	    : xag(xag), to_qubit(xag, WireRef::invalid()), ltfi(xag),
	      circuit(circuit)
	{}
};

void preprocess_inputs(State& state, std::vector<WireRef> const& qubits)
{
	uint32_t index = 0;
	state.xag.foreach_pi([&](XAGNode const& node) {
		// fmt::print("{}\n", index);
		state.to_qubit[node] = qubits[index];
		state.ltfi[node].emplace_back(state.xag.make_signal(node));
		index += 1;
	});
}

void preprocess_outputs(State& state, std::vector<WireRef> const& qubits)
{
	uint32_t index = state.xag.num_pis();
	state.xag.foreach_po([&](XAGSignal const& signal) {
		XAGNode const& node = state.xag.get_node(signal);
		if (state.xag.is_and(node)) {
			if (state.to_qubit[signal] == WireRef::invalid()) {
				state.to_qubit[signal] = qubits[index];
			}
		}
		index += 1;
	});
	index = state.xag.num_pis();
	state.xag.foreach_po([&](XAGSignal const& signal) {
		XAGNode const& node = state.xag.get_node(signal);
		if (!state.xag.is_xor(node)) {
			index += 1;
			return;
		}
		if (state.to_qubit[signal] == WireRef::invalid()) {
			state.to_qubit[signal] = qubits[index];
		}
		auto test = state.ltfi[node];
		std::reverse(test.begin(), test.end());
		// for (XAGSignal in : state.ltfi[node]) {
		for (XAGSignal in : test) {
			if (!(state.to_qubit[in] == WireRef::invalid())) {
				continue;
			}
			XAGNode in_node = state.xag.get_node(in);
			if (!state.xag.is_and(in_node)) {
				continue;
			}
			state.to_qubit[signal] = qubits[index];
			state.to_qubit[in_node] = qubits[index];
			break;
		}
		index += 1;
	});
}

void compute_ltfi(
    XAG const& xag, XAGNode const& node, mockturtle::node_map<LTFI, XAG>& ltfi)
{
	if (xag.is_and(node)) {
		ltfi[node].emplace_back(xag.make_signal(node));
		return;
	}
	std::array<XAGSignal, 2> fanins;
	xag.foreach_fanin(node, [&](XAGSignal const& signal, uint32_t i) {
		fanins.at(i) = signal;
	});
	auto const& ltfi_in0 = ltfi[fanins.at(0)];
	auto const& ltfi_in1 = ltfi[fanins.at(1)];
	assert(!ltfi_in0.empty());
	assert(!ltfi_in1.empty());
	std::set_symmetric_difference(ltfi_in0.begin(), ltfi_in0.end(),
	    ltfi_in1.begin(), ltfi_in1.end(), std::back_inserter(ltfi[node]));
	assert(
	    ltfi[node].size()
	    && "If the size is empty, then the XAG was not properly optimized!");
}

uint32_t analyze_xag(XAG const& xag, mockturtle::node_map<LTFI, XAG>& ltfi)
{
	uint32_t num_ands = 0u;
	xag.clear_values();
	xag.foreach_gate([&](XAGNode const& node) {
		compute_ltfi(xag, node, ltfi);
		if (xag.is_and(node)) {
			num_ands += 1;
		}
	});
	return num_ands;
}

void compute_inputs(State& state, XAGNode const& node, ParityAnd& gate)
{
	std::array<XAGSignal, 2> fanins;
	bool both_xor = true;
	state.xag.foreach_fanin(
	    node, [&](XAGSignal const signal, uint32_t index) {
		    XAGNode const& fanin = state.xag.get_node(signal);
		    both_xor &= state.xag.is_xor(fanin);
		    fanins.at(index) = signal;
	    });

	LTFI const* ltfi_0 = &(state.ltfi[fanins.at(0)]);
	LTFI const* ltfi_1 = &(state.ltfi[fanins.at(1)]);
	if ((ltfi_0->size() == 1) && (ltfi_1->size() == 1)) {
		gate.in0.push_back(state.to_qubit[ltfi_0->at(0)]);
		gate.in1.push_back(state.to_qubit[ltfi_1->at(0)]);
		return;
	}
	// Normalization
	if (ltfi_0->size() < ltfi_1->size()) {
		std::swap(ltfi_0, ltfi_1);
		std::swap(fanins.at(0), fanins.at(1));
	}
	//
	auto in0_begin = ltfi_0->cbegin();
	auto in0_end = ltfi_0->cend();
	auto in1_begin = ltfi_1->cbegin();
	auto in1_end = ltfi_1->cend();
	while (in0_begin != in0_end && in1_begin != in1_end) {
		if (*in0_begin == *in1_begin) {
			gate.in01.emplace_back(state.to_qubit[*in0_begin]);
			++in0_begin;
			++in1_begin;
		} else if (*in0_begin < *in1_begin) {
			gate.in0.emplace_back(state.to_qubit[*in0_begin]);
			++in0_begin;
		} else {
			gate.in1.emplace_back(state.to_qubit[*in1_begin]);
			++in1_begin;
		}
	}
	while (in0_begin != in0_end) {
		gate.in0.emplace_back(state.to_qubit[*in0_begin]);
		++in0_begin;
	}
	while (in1_begin != in1_end) {
		gate.in1.emplace_back(state.to_qubit[*in1_begin]);
		++in1_begin;
	}
}

void compute_gate(
    State& state, XAGNode const& node, std::vector<ParityAnd>& gates)
{
	// Compute inputs
	ParityAnd& gate = gates.emplace_back();
	compute_inputs(state, node, gate);
	WireRef target = state.to_qubit[node];
	if (target == WireRef::invalid()) {
		target = state.circuit.request_ancilla();
		gate.cleanup = true;
	}
	gate.target = target;
	state.to_qubit[node] = target;
	state.xag.foreach_fanin(node,
	[&](XAGSignal const& signal, uint32_t i) {
		if (state.xag.is_complemented(signal)) {
			gate.is_complemented[i] = true;
		} 
	});
}

void choose_target(std::vector<uint32_t>& usage, std::vector<WireRef>& qubits)
{
	if (qubits.size() == 1) {
		return;
	}
	uint32_t chosen_qubit = 0;
	uint32_t max = usage.at(qubits[chosen_qubit].uid());
	for (uint32_t i = 1; i < qubits.size(); ++i) {
		if (max < usage.at(qubits[i].uid())) {
			chosen_qubit = i;
			max = usage.at(qubits[i].uid());
		}
	}
	usage.at(qubits[chosen_qubit].uid()) += 1;
	std::swap(qubits[chosen_qubit], qubits.back());
}

InstRef compute_parity(Circuit& circuit, std::vector<WireRef> const& qubits)
{
	if (qubits.size() == 1) {
		return InstRef::invalid();
	}
	return circuit.create_instruction(GateLib::Parity(), {qubits});
}

void synthesize(
    Circuit& circuit, std::vector<WireRef> const& qubits, XAG const& xag)
{
	State state(xag, circuit);
	preprocess_inputs(state, qubits);
	uint32_t const num_ands = analyze_xag(state.xag, state.ltfi);
	preprocess_outputs(state, qubits);

	// Create a Network with ParityAnd gates
	std::vector<ParityAnd> gates;
	gates.reserve(num_ands);
	xag.foreach_gate([&](XAGNode const& node) {
		if (!xag.is_and(node)) {
			return;
		}
		compute_gate(state, node, gates);
	});

	 std::vector<uint32_t> usage(circuit.num_wires(), 0);
	// Create a network with Parity and Toffoli gates
	for (ParityAnd& gate : gates) {
		// Compute inputs
		choose_target(usage, gate.in0);
		compute_parity(circuit, gate.in0);
		if (!gate.in01.empty()) {
			choose_target(usage, gate.in01);
			compute_parity(circuit, gate.in01);
			gate.in1.push_back(gate.in01.back());
			circuit.create_instruction(GateLib::X(), {gate.in01.back()}, gate.in0.back());
		} else {
			choose_target(usage, gate.in1);
		}
		compute_parity(circuit, gate.in1);
		// Compute Toffoli
		WireRef c0 = gate.is_complemented[0] ? !gate.in0.back() : gate.in0.back();
		WireRef c1 = gate.is_complemented[1] ? !gate.in1.back() : gate.in1.back();
		circuit.create_instruction(GateLib::X(), {c0, c1}, gate.target);
		compute_parity(circuit, gate.in1);
		if (!gate.in01.empty()) {
			circuit.create_instruction(GateLib::X(), {gate.in01.back()}, gate.in0.back());
			compute_parity(circuit, gate.in01);
		}
		compute_parity(circuit, gate.in0);
	}
	// Compute XOR POs
	xag.foreach_po([&](XAGSignal const& signal) {
		XAGNode const& node = xag.get_node(signal);

		if (!xag.is_xor(node) || state.to_qubit[signal] == WireRef::invalid()) {
			return;
		}
		if (xag.visited(node)) {
			return;
		}
		xag.set_visited(node, 1);
		std::vector<WireRef> qubits;
		for (XAGSignal const& in : state.ltfi[node]) {
			if (state.to_qubit[in] == state.to_qubit[signal]) {
				continue;
			}
			qubits.push_back(state.to_qubit[in]);
		}
		qubits.push_back(state.to_qubit[signal]);
		compute_parity(circuit, qubits);
	});
	uint32_t index = xag.num_pis();
	xag.foreach_po([&](XAGSignal const& signal) {
		XAGNode const& node = xag.get_node(signal);
		if (xag.is_constant(node)) {
			index += 1;
			return;
		}
		if (xag.is_pi(node)) {
			circuit.create_instruction(GateLib::X(), {state.to_qubit[signal]}, qubits[index]);
			index += 1;
			return;
		}
		if (xag.visited(node) == 2) {
			circuit.create_instruction(GateLib::X(), {state.to_qubit[signal]}, qubits[index]);
		}
		xag.set_visited(node, 2);
		index += 1;
	});
	index = xag.num_pis();
	xag.foreach_po([&](XAGSignal const& signal) {
		if (xag.is_complemented(signal)) {
			circuit.create_instruction(GateLib::X(), {qubits[index]});
		}
		index += 1;
	});
	// // Cleanup
	// auto begin = gates.crbegin();
	// auto end = gates.crend();
	// for (;begin != end; ++begin) {
	// 	if (begin->cleanup == false) {
	// 		continue;
	// 	}
	// 	compute_parity(circuit, begin->in1);
	// 	if (!begin->in01.empty()) {
	// 		compute_parity(circuit, begin->in01);
	// 		circuit.create_instruction(GateLib::X(), {begin->in01.back()}, begin->in0.back());
	// 	}
	// 	compute_parity(circuit, begin->in0);
	// 	WireRef c0 = begin->is_complemented[0] ? !begin->in0.back() : begin->in0.back();
	// 	WireRef c1 = begin->is_complemented[1] ? !begin->in1.back() : begin->in1.back();
	// 	circuit.create_instruction(GateLib::X(), {c0, c1}, begin->target);
	// 	compute_parity(circuit, begin->in1);
	// 	if (!begin->in01.empty()) {
	// 		circuit.create_instruction(GateLib::X(), {begin->in01.back()}, begin->in0.back()); 
	// 		compute_parity(circuit, begin->in01);
	// 	}
	// 	compute_parity(circuit, begin->in0);
	// }
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