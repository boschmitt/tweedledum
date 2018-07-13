/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include "../representations/gate_kinds.hpp"

#include <tweedledee/dotqc/dotqc.hpp>

namespace tweedledum {

struct identify_gate_kind {
	gate_kinds operator()(std::string_view gate_label)
	{
		switch (gate_label[0]) {
		case 'H':
			return gate_kinds::hadamard;

		case 'S':
		case 'P':
			if (gate_label.size() == 2 && gate_label[1] == '*')
				return gate_kinds::phase_dagger;
			return gate_kinds::phase;

		case 'T':
			if (gate_label.size() == 2 && gate_label[1] == '*')
				return gate_kinds::t_dagger;
			return gate_kinds::t;

		case 'X':
			return gate_kinds::pauli_x;

		case 'Y':
			return gate_kinds::pauli_y;

		case 'Z':
			return gate_kinds::pauli_z;

		default:
			break;
		}
		if (gate_label == "tof") {
			return gate_kinds::cnot;
		}
		return gate_kinds::unknown;
	}
};

template<typename R>
class dotqc_reader : public tweedledee::dotqc_reader<gate_kinds> {
public:
	explicit dotqc_reader(R& representation)
	    : representation_(representation)
	{}

	void on_qubit(std::string qubit_label)
	{
		representation_.add_qubit(qubit_label);
	}

	void on_input(std::string qubit_label)
	{
		representation_.mark_as_input(qubit_label);
	}

	void on_output(std::string qubit_label)
	{
		representation_.mark_as_output(qubit_label);
	}

	void on_gate(gate_kinds kind, std::string qubit_label)
	{
		representation_.add_gate(kind, qubit_label);
	}

	void on_two_qubit_gate(gate_kinds kind, std::string qubit0_label,
	                       std::string qubit1_label)
	{
		representation_.add_controlled_gate(kind, qubit0_label, qubit1_label);
	}

private:
	R& representation_;
};

} // namespace tweedledum
