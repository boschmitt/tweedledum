/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_base.hpp"
#include "../gates/gate_set.hpp"
#include "../networks/io_id.hpp"

#include <cassert>
#include <tweedledee/dotqc/dotqc.hpp>

namespace tweedledum {

struct identify_gate {
	gate_base operator()(std::string_view gate_label)
	{
		switch (gate_label[0]) {
		case 'H':
			return gate::hadamard;

		case 'S':
		case 'P':
			if (gate_label.size() == 2 && gate_label[1] == '*') {
				return gate::phase_dagger;
			}
			return gate::phase;

		case 'T':
			if (gate_label.size() == 2 && gate_label[1] == '*')
				return gate::t_dagger;
			return gate::t;

		case 'X':
			return gate::pauli_x;

		case 'Y':
			return gate::pauli_y;

		case 'Z':
			return gate::pauli_z;

		default:
			break;
		}
		if (gate_label == "tof") {
			return gate::cx;
		}
		return gate_base(gate_set::unknown);
	}
};

template<typename Network>
class dotqc_reader : public tweedledee::dotqc_reader<gate_base> {
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

	void on_gate(gate_base gate, std::string const& target)

	{
		network_.add_gate(gate, target);
	}

	void on_gate(gate_base gate, std::vector<std::string> const& controls,
	             std::vector<std::string> const& targets)
	{
		switch (gate.operation()) {
		case gate_set::pauli_x:
			if (controls.size() == 1) {
				gate = gate::cx;
			} else if (controls.size() >= 2) {
				gate = gate::mcx;
			}
			break;

		case gate_set::pauli_z:
			if (controls.size() == 1) {
				gate = gate::cz;
			} else if (controls.size() >= 2) {
				gate = gate::mcz;
			}
			break;

		default:
			break;
		}
		network_.add_gate(gate, controls, targets);
	}

private:
	Network& network_;
};

} // namespace tweedledum
