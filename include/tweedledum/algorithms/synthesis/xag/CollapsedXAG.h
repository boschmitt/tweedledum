/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <cassert>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <vector>

namespace tweedledum::xag_synth_detail {

// A gate it's either an input or a ParityAnd---a two input AND gate which the
// inputs are parity functions.
struct Gate {
	std::vector<uint32_t> in0;
	std::vector<uint32_t> in1;
	std::vector<uint32_t> in01;
	std::array<bool, 2> is_negated;

	// Metadata
	uint32_t is_and : 1;
	uint32_t ref_count : 31;
	uint32_t cleanup;

	Gate(bool is_and = true) : is_and(is_and), ref_count(0), cleanup(is_and)
	{}
};

struct Output {
	std::vector<uint32_t> fanin;
	uint32_t is_negated : 1;
	uint32_t cleanup : 31;
	uint32_t num_ands;
	Output(bool is_negated = false)
	    : is_negated(is_negated), cleanup(0), num_ands(0)
	{}
};

struct CollapsedXAG {
	std::vector<Gate> gates;
	std::vector<Output> outputs;
	uint32_t num_inputs;

	CollapsedXAG(uint32_t num_gates, uint32_t num_inputs)
	    : num_inputs(num_inputs)
	{
		gates.reserve(num_gates);
		create_input(); // This is the constant gate
		for (uint32_t i = 0; i < num_inputs; ++i) {
			create_input();
		}
	}

	Gate& create_gate()
	{
		return gates.emplace_back();
	}

	Gate& create_input()
	{
		return gates.emplace_back(/* is_and */ false);
	}

	Output& create_output(bool is_negated)
	{
		return outputs.emplace_back(is_negated);
	}

	void incr_references(uint32_t index)
	{
		gates.at(index).ref_count += 1;
	}

	// Returns the number of gates in the circuit
	uint32_t size() const
	{
		return gates.size();
	}
};

CollapsedXAG collapse_xag(mockturtle::xag_network const& xag)
{
	using XAG = mockturtle::xag_network;
	using Node = typename XAG::node;
	using Signal = typename XAG::signal;
	using LTFI = std::vector<Signal>;

	// Preprocess:
	// - Compute LTFI
	mockturtle::node_map<LTFI, XAG> ltfi(xag);
	uint32_t new_index = 1;
	xag.foreach_pi([&](Node const& node) {
		ltfi[node].emplace_back(xag.make_signal(node));
		xag.set_value(node, new_index);
		new_index += 1;
		return;
	});
	xag.foreach_gate([&](Node const& node) {
		std::array<LTFI const*, 2> node_ltfi;
		xag.foreach_fanin(node, [&](Signal const signal, uint32_t i) {
			node_ltfi[i] = &(ltfi[signal]);
		});
		if (xag.is_and(node)) {
			ltfi[node].emplace_back(xag.make_signal(node));
			xag.set_value(node, new_index);
			new_index += 1;
			return;
		}
		// The node is then a XOR
		std::set_symmetric_difference(node_ltfi[0]->cbegin(),
		    node_ltfi[0]->cend(), node_ltfi[1]->cbegin(),
		    node_ltfi[1]->cend(), std::back_inserter(ltfi[node]));
		assert(ltfi[node].size());
	});

	// Create the collapsed XAG
	CollapsedXAG collapsed_xag(new_index, xag.num_pis());
	xag.foreach_gate([&](Node const& node) {
		if (!xag.is_and(node)) {
			return;
		}
		auto& gate = collapsed_xag.create_gate();
		std::array<LTFI const*, 2> node_ltfi;
		xag.foreach_fanin(node, [&](Signal const signal, uint32_t i) {
			node_ltfi[i] = &(ltfi[signal]);
			gate.is_negated[i] = xag.is_complemented(signal);
		});
		auto in0_begin = node_ltfi[0]->cbegin();
		auto in0_end = node_ltfi[0]->cend();
		auto in1_begin = node_ltfi[1]->cbegin();
		auto in1_end = node_ltfi[1]->cend();
		while (in0_begin != in0_end && in1_begin != in1_end) {
			if (*in0_begin == *in1_begin) {
				uint32_t id = xag.value(xag.get_node(*in0_begin));
				gate.in01.emplace_back(id);
				collapsed_xag.incr_references(id);
				++in0_begin;
				++in1_begin;
			} else if (*in0_begin < *in1_begin) {
				uint32_t id = xag.value(xag.get_node(*in0_begin));
				gate.in0.emplace_back(id);
				collapsed_xag.incr_references(id);
				++in0_begin;
			} else {
				uint32_t id = xag.value(xag.get_node(*in1_begin));
				gate.in1.emplace_back(id);
				collapsed_xag.incr_references(id);
				++in1_begin;
			}
		}
		while (in0_begin != in0_end) {
			uint32_t id = xag.value(xag.get_node(*in0_begin));
			gate.in0.emplace_back(id);
			collapsed_xag.incr_references(id);
			++in0_begin;
		}
		while (in1_begin != in1_end) {
			uint32_t id = xag.value(xag.get_node(*in1_begin));
			gate.in1.emplace_back(id);
			collapsed_xag.incr_references(id);
			++in1_begin;
		}
		if (gate.in0.size() < gate.in1.size()) {
			std::swap(gate.in0, gate.in1);
			std::swap(gate.is_negated[0], gate.is_negated[1]);
		}
	});
	xag.foreach_po([&](Signal const& signal) {
		auto& output
		    = collapsed_xag.create_output(xag.is_complemented(signal));
		for (Signal const input : ltfi[signal]) {
			uint32_t id = xag.value(xag.get_node(input));
			output.fanin.emplace_back(id);
			collapsed_xag.incr_references(id);
			// This shouldn't be here:
			collapsed_xag.gates[id].cleanup = 2;
			if (collapsed_xag.gates[id].is_and) {
				output.num_ands += 1;
			}
		}
	});
	return collapsed_xag;
}

} // namespace tweedledum::xag_synth_detail