/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdio>

#include "../representations/gate_kinds.hpp"

namespace tweedledum {

template<typename Circuit>
static void write_qpic(Circuit& circuit, std::string filename = "test.qpic")
{
	auto file = fopen(filename.c_str(), "w");

	circuit.foreach_qubit([&file] (auto id, auto& name) {
		fprintf(file, "q%d W %s %s\n", id, name.c_str(), name.c_str());
	});

	circuit.foreach_gate([&file] (auto node) {
		switch (node.gate.kind()) {
		case gate_kinds::cnot:
			fprintf(file, "q%d +q%d\n", node.gate.control(), node.gate.target());
			break;

		case gate_kinds::hadamard:
			fprintf(file, "q%d H\n", node.gate.target());
			break;

		case gate_kinds::phase:
			fprintf(file, "q%d G $P$\n", node.gate.target());
			break;

		case gate_kinds::phase_dagger:
			fprintf(file, "q%d G $P^{\\dagger}$\n", node.gate.target());
			break;

		case gate_kinds::t:
			fprintf(file, "q%d G $T$\n", node.gate.target());
			break;

		case gate_kinds::t_dagger:
			fprintf(file, "q%d G $T^{\\dagger}$\n", node.gate.target());
			break;

		default:
			break;
		}
	});
	fclose(file);
}

} // namespace tweedledum

