/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../ir/Circuit.h"
#include "../../../ir/GateLib.h"
#include "../../../ir/Wire.h"
#include "BennettStrategy.h"
#include "EagerStrategy.h"

#include <mockturtle/algorithms/collapse_mapped.hpp>
#include <mockturtle/algorithms/lut_mapping.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/mapping_view.hpp>

namespace tweedledum {

namespace detail {

template<typename LogicNetwork>
inline mockturtle::klut_network collapse_to_klut(LogicNetwork const& network)
{
	using namespace mockturtle;
	// lut_mapping_params ps;
	// ps.cut_enumeration_ps.cut_size = 8;
	// Do LUT mapping while storing the functions.
	mapping_view<LogicNetwork, true> mapped_network(network);
	lut_mapping<mapping_view<LogicNetwork, true>, true>(mapped_network);
	// Collapse and return
	return *collapse_mapped_network<klut_network>(mapped_network);
}

class LHRSynth {
	using LogicNetwork = mockturtle::klut_network;
	using Action = BaseStrategy::Action;

public:
	static void synthesize(LogicNetwork const& klut,
	    BaseStrategy const& strategy, Circuit& circuit,
	    std::vector<WireRef> const& qubits);

private:
	LHRSynth(LogicNetwork const& klut, BaseStrategy const& strategy,
	    Circuit& circuit, std::vector<WireRef> const& qubits)
	    : klut_(klut), strategy_(strategy), circuit_(circuit),
	      qubits_(qubits), to_qubit_(klut, WireRef::invalid())
	{}

	void do_synthesize();

	LogicNetwork const& klut_;
	BaseStrategy const& strategy_;
	Circuit& circuit_;
	std::vector<WireRef> const& qubits_;
	mockturtle::node_map<WireRef, LogicNetwork> to_qubit_;
};

inline void LHRSynth::do_synthesize()
{
	uint32_t i = 0u;
	klut_.foreach_pi([&](auto const& node) {
		to_qubit_[node] = qubits_.at(i++);
	});

	// Analysis of the primary outputs:  Here I do two things:
	// *) look for primary outputs that point to the same node.  For those
	//     we need to only compute one and then at the end use a CX to copy
	//     the computational state.
	// *) check which outputs will need be complemented at the end.
	klut_.clear_visited();
	std::vector<uint32_t> to_compute_po;
	std::vector<uint32_t> to_complement_po;
	klut_.foreach_po([&](auto const& signal) {
		auto node = klut_.get_node(signal);
		if (!klut_.visited(node)) {
			to_qubit_[node] = qubits_.at(i);
			klut_.set_visited(node, 1u);
			if (klut_.is_complemented(signal)) {
				to_complement_po.push_back(i);
			}
		} else {
			to_compute_po.push_back(i);
		}
		++i;
	});

	// Perform the action of all the steps.
	for (auto const& step : strategy_) {
		std::vector<WireRef> controls;
		klut_.foreach_fanin(step.node, [&](auto const& signal) {
			WireRef qubit = to_qubit_[klut_.get_node(signal)];
			if (klut_.is_complemented(signal)) {
				controls.emplace_back(!qubit);
			} else {
				controls.emplace_back(qubit);
			}
		});
		switch (step.action) {
		case Action::compute:
			if (to_qubit_[step.node] == WireRef::invalid()) {
				to_qubit_[step.node] = circuit_.request_ancilla();
			}
			break;

		case Action::cleanup:
			circuit_.release_ancilla(to_qubit_[step.node]);
			break;
		}
		circuit_.create_instruction(
		    GateLib::TruthTable("", klut_.node_function(step.node)),
		    controls, to_qubit_[step.node]);
	}

	// Compute the outputs that need to be "copied" from other qubits.
	for (uint32_t po : to_compute_po) {
		auto const signal = klut_.po_at(po - klut_.num_pis());
		auto const node = klut_.get_node(signal);
		WireRef qubit = to_qubit_[node];
		if (klut_.is_complemented(signal)) {
			qubit.complement();
		}
		circuit_.create_instruction(
		    GateLib::X(), {qubit}, qubits_.at(po));
	}
	// Complement what needs to be complemented.
	for (uint32_t po : to_complement_po) {
		auto const signal = klut_.po_at(po - klut_.num_pis());
		auto const node = klut_.get_node(signal);
		WireRef const qubit = to_qubit_[node];
		circuit_.create_instruction(GateLib::X(), {qubit});
	}
}

inline void LHRSynth::synthesize(LogicNetwork const& klut,
    BaseStrategy const& strategy, Circuit& circuit,
    std::vector<WireRef> const& qubits)
{
	LHRSynth synth(klut, strategy, circuit, qubits);
	synth.do_synthesize();
}

} // namespace detail

inline void lhrs_synth(mockturtle::klut_network const& klut, Circuit& circuit,
    std::vector<WireRef> const& qubits)
{
	BennettStrategy strategy;
	strategy.compute_steps(klut);
	detail::LHRSynth::synthesize(klut, strategy, circuit, qubits);
}

//  LUT-based hierarchical reversible logic synthesis (LHRS)
template<typename LogicNetwork>
inline Circuit lhrs_synth(LogicNetwork const& network)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");

	auto const klut = detail::collapse_to_klut(network);
	// Create the necessary qubits
	uint32_t num_qubits = klut.num_pis() + klut.num_pos();
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	lhrs_synth(klut, circuit, wires);
	return circuit;
}

} // namespace tweedledum
