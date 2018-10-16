/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <tweedledee/dotqc/dotqc.hpp>

namespace tweedledum {

struct identify_gate_kind {
	gate_kinds_t operator()(std::string_view gate_label)
	{
		switch (gate_label[0]) {
		case 'H':
			return gate_kinds_t::hadamard;

		case 'S':
		case 'P':
			if (gate_label.size() == 2 && gate_label[1] == '*')
				return gate_kinds_t::phase_dagger;
			return gate_kinds_t::phase;

		case 'T':
			if (gate_label.size() == 2 && gate_label[1] == '*')
				return gate_kinds_t::t_dagger;
			return gate_kinds_t::t;

		case 'X':
			return gate_kinds_t::pauli_x;

		case 'Y':
			return gate_kinds_t::pauli_y;

		case 'Z':
			return gate_kinds_t::pauli_z;

		default:
			break;
		}
		if (gate_label == "tof") {
			return gate_kinds_t::cx;
		}
		return gate_kinds_t::unknown;
	}
};

template<typename Network>
class dotqc_reader : public tweedledee::dotqc_reader<gate_kinds_t> {
public:
	explicit dotqc_reader(Network& network)
	    : network_(network)
	{}

	void on_qubit(std::string qubit_label)
	{
		network_.add_qubit(qubit_label);
	}

	void on_input(std::string qubit_label)
	{
		(void) qubit_label;
		// network_.mark_as_input(qubit_label);
	}

	void on_output(std::string qubit_label)
	{
		(void) qubit_label;
		// network_.mark_as_output(qubit_label);
	}

	void on_gate(gate_kinds_t kind, std::string const& target)
	{
		network_.add_gate(kind, target);
	}

	void on_gate(gate_kinds_t kind, std::vector<std::string> const& controls,
	             std::vector<std::string> const& targets)
	{
		switch (kind) {
		case gate_kinds_t::pauli_x:
			if (controls.size() == 1) {
				kind = gate_kinds_t::cx;
			} else if (controls.size() >= 2) {
				kind = gate_kinds_t::mcx;
			}
			break;

		case gate_kinds_t::pauli_z:
			if (controls.size() == 1) {
				kind = gate_kinds_t::cz;
			} else if (controls.size() >= 2) {
				kind = gate_kinds_t::mcz;
			}
			break;

		default:
			break;
		}
		network_.add_gate(kind, controls, targets);
	}

private:
	Network& network_;
};

} // namespace tweedledum
