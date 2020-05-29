/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../ir/Circuit.h"
#include "../../../ir/GateLib.h"
#include "../../../ir/Wire.h"
#include "BennettStrategy.h"

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

} // namespace detail

inline void lhrs_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    mockturtle::klut_network const& klut)
{
	using LogicNtk = mockturtle::klut_network;
	using Action = BaseStrategy::Action;
	using namespace mockturtle;

	BennettStrategy strategy;
	strategy.compute_steps(klut);

	mockturtle::node_map<WireRef, LogicNtk> to_qubit(
	    klut, WireRef::invalid());
	uint32_t i = 0u;
	klut.foreach_pi([&](auto const& node) {
		to_qubit[node] = qubits.at(i++);
	});
	klut.clear_visited();
	std::vector<uint32_t> to_compute_po;
	std::vector<uint32_t> to_complement_po;
	klut.foreach_po([&](auto const& signal) {
		auto node = klut.get_node(signal);
		if (!klut.visited(node)) {
			to_qubit[node] = qubits.at(i);
			klut.set_visited(node, 1u);
			if (klut.is_complemented(signal)) {
				to_complement_po.push_back(i);
			}
		} else {
			to_compute_po.push_back(i);
		}
		++i;
	});

	for (auto const& step : strategy) {
		std::vector<WireRef> controls;
		klut.foreach_fanin(step.node, [&](auto const& signal) {
			WireRef qubit = to_qubit[klut.get_node(signal)];
			if (klut.is_complemented(signal)) {
				controls.emplace_back(!qubit);
			} else {
				controls.emplace_back(qubit);
			}
		});
		switch (step.action) {
		case Action::compute:
			if (to_qubit[step.node] == WireRef::invalid()) {
				to_qubit[step.node] = circuit.create_qubit();
			}
			break;

		case Action::cleanup:
			break;
		}
		circuit.create_instruction(
		    GateLib::TruthTable("", klut.node_function(step.node)),
		    controls, to_qubit[step.node]);
	}
	// Finalize
	for (uint32_t po : to_compute_po) {
		auto const signal = klut.po_at(po - klut.num_pis());
		auto const node = klut.get_node(signal);
		WireRef qubit = to_qubit[node];
		if (klut.is_complemented(signal)) {
			qubit.complement();
		}
		circuit.create_instruction(GateLib::X(), {qubit}, qubits.at(po));
	}
	for (uint32_t po : to_complement_po) {
		auto const signal = klut.po_at(po - klut.num_pis());
		auto const node = klut.get_node(signal);
		WireRef const qubit = to_qubit[node];
		circuit.create_instruction(GateLib::X(), {qubit});
	}
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
	lhrs_synth(circuit, wires, klut);
	return circuit;
}

} // namespace tweedledum
