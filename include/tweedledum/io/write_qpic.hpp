/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <cstdio>

namespace tweedledum {

template<typename Network>
static void write_qpic(Network const& network, std::string filename = "test.qpic",
                       bool color_marked_gates = false)
{
	auto file = fopen(filename.c_str(), "w");

	if (color_marked_gates) {
		fprintf(file, "DEFINE mark color=red:style=thick\n");
	}
	network.foreach_qubit([&file](auto id, auto& name) {
		fprintf(file, "q%d W %s %s\n", id, name.c_str(), name.c_str());
	});

	fprintf(file, "\n");
	network.foreach_gate([&](auto& node) {
		// This is anoying :(
		if (node.gate.is(gate_kinds_t::mcx)) {
			node.gate.foreach_target([&](auto qubit) { fprintf(file, "+q%d ", qubit); });
		} else {
			node.gate.foreach_target([&](auto qubit) { fprintf(file, "q%d ", qubit); });
		}
		switch (node.gate.kind()) {
		case gate_kinds_t::pauli_x:
			fprintf(file, "N");
			break;

		case gate_kinds_t::cx:
			fprintf(file, "C");
			break;

		case gate_kinds_t::mcx:
			break;

		case gate_kinds_t::pauli_z:
		case gate_kinds_t::cz:
		case gate_kinds_t::mcz:
			fprintf(file, "Z");
			break;

		case gate_kinds_t::hadamard:
			fprintf(file, "H");
			break;

		case gate_kinds_t::phase:
			fprintf(file, "G $P$");
			break;

		case gate_kinds_t::phase_dagger:
			fprintf(file, "G $P^{\\dagger}$");
			break;

		case gate_kinds_t::t:
			fprintf(file, "G $T$");
			break;

		case gate_kinds_t::t_dagger:
			fprintf(file, "G $T^{\\dagger}$");
			break;

		case gate_kinds_t::rotation_x:
			fprintf(file, "G $R_{x}$");
			break;

		case gate_kinds_t::rotation_z:
			fprintf(file, "G $R_{z}$");
			break;

		default:
			break;
		}
		node.gate.foreach_control([&](auto qubit) { fprintf(file, " q%d", qubit); });
		fprintf(file, "%s", color_marked_gates && network.mark(node) ? " mark\n" : "\n");
	});
	fclose(file);
}

} // namespace tweedledum
