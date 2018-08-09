/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../networks/gates/gate_kinds.hpp"

#include <cstdio>

namespace tweedledum {

template<typename Circuit>
static void write_qpic(Circuit& circuit, std::string filename = "test.qpic",
                       bool color_marked_gates = false)
{
	auto file = fopen(filename.c_str(), "w");

	if (color_marked_gates) {
		fprintf(file, "DEFINE mark color=red:style=thick\n");
	}
	circuit.foreach_qubit([&file](auto id, auto& name) {
		fprintf(file, "q%d W %s %s\n", id, name.c_str(), name.c_str());
	});

	fprintf(file, "\n");
	circuit.foreach_gate([&](auto node) {
		switch (node.gate.kind()) {
		case gate_kinds_t::pauli_x:
		case gate_kinds_t::cx:
		case gate_kinds_t::mcx:
			node.gate.foreach_control(
			    [&](auto qubit) { fprintf(file, "q%d ", qubit); });
			node.gate.foreach_target(
			    [&](auto qubit) { fprintf(file, "+q%d ", qubit); });
			break;

		case gate_kinds_t::pauli_z:
		case gate_kinds_t::cz:
		case gate_kinds_t::mcz:
			node.gate.foreach_target(
			    [&](auto qubit) { fprintf(file, "q%d ", qubit); });
			fprintf(file, "Z ");
			node.gate.foreach_control(
			    [&](auto qubit) { fprintf(file, " q%d", qubit); });
			break;

		case gate_kinds_t::hadamard:
			fprintf(file, "q%d H", node.gate.target());
			break;

		case gate_kinds_t::phase:
			fprintf(file, "q%d G $P$", node.gate.target());
			break;

		case gate_kinds_t::phase_dagger:
			fprintf(file, "q%d G $P^{\\dagger}$", node.gate.target());
			break;

		case gate_kinds_t::t:
			fprintf(file, "q%d G $T$", node.gate.target());
			break;

		case gate_kinds_t::t_dagger:
			fprintf(file, "q%d G $T^{\\dagger}$", node.gate.target());
			break;

		case gate_kinds_t::rotation_x:
			fprintf(file, "q%d G $R_{x}$", node.gate.target());
			break;

		case gate_kinds_t::rotation_z:
			fprintf(file, "q%d G $R_{z}$", node.gate.target());
			break;

		default:
			break;
		}
		fprintf(file, "%s",
		        color_marked_gates && circuit.mark(node) ? " mark\n" :
		                                                   "\n");
	});
	fclose(file);
}

} // namespace tweedledum
