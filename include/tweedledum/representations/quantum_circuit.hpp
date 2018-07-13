/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include "gate_kinds.hpp"

#include <iostream>
#include <string>

namespace tweedledum {

class quantum_circuit {
public:
	void add_qubit(std::string const& qubit = {})
	{
		std::cout << "Add qubit: " << qubit << '\n';
	}

	void mark_as_input(std::string const& qubit)
	{
		std::cout << "Mark as input: " << qubit << '\n';
	}

	void mark_as_output(std::string const& qubit)
	{
		std::cout << "Mark as output: " << qubit << '\n';
	}

	void add_gate(gate_kinds kind, std::string const& target)
	{
		std::cout << "Add " << gate_name(kind)
		          << " gate to qubit: " << target << '\n';
	}

	void add_controlled_gate(gate_kinds kind, std::string const& control,
	                         std::string const& target)
	{
		std::cout << "Add " << gate_name(kind)
		          << " gate to qubits: " << control << ", " << target
		          << '\n';
	}
};

} // namespace tweedledum
