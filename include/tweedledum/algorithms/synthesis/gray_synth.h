/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/Matrix.h"
#include "cnot_synth.h"

namespace tweedledum {
namespace gray_synth_detail {

using AbstractGate = std::pair<uint32_t, uint32_t>;
using GateList = std::vector<AbstractGate>;

struct State {
	std::vector<uint32_t> sel_cols;
	std::vector<uint32_t> rem_rows;
	uint32_t qubit;

	State(std::vector<uint32_t> const& sel_cols,
	    std::vector<uint32_t> const& rem_rows, uint32_t qubit)
	    : sel_cols(sel_cols), rem_rows(rem_rows), qubit(qubit)
	{}
};

template<typename T>
uint32_t select_row(State& state, Matrix<T> const& matrix)
{
	assert(!state.rem_rows.empty());
	uint32_t sel_row = 0;
	uint32_t max = 0;

	for (uint32_t row_idx : state.rem_rows) {
		auto row = matrix.row(row_idx);
		uint32_t num_ones = row.sum();
		uint32_t num_zeros = matrix.num_columns() - num_ones;
		uint32_t local_max = std::max(num_ones, num_zeros);
		if (local_max > max) {
			max = local_max;
			sel_row = row_idx;
		}
	}
	return sel_row;
}

template<typename T>
inline void add_gate(State& state, Matrix<T>& matrix, GateList& gates)
{
	for (uint32_t j = 0u; j < matrix.num_rows(); ++j) {
		if (j == state.qubit) {
			continue;
		}
		bool all_one = true;
		for (uint32_t col : state.sel_cols) {
			all_one &= (matrix(j, col) == 1);
		}
		if (!all_one) {
			continue;
		}
		matrix.row(j) ^= std::valarray(matrix.row(state.qubit));
		gates.emplace_back(j, state.qubit);
	}
}

template<typename T>
GateList synthesize(std::vector<WireRef> const& qubits, Matrix<T>& matrix)
{
	GateList gates;
	uint32_t const num_qubits = qubits.size();

	// Initial state
	std::vector<State> state_stack(1, {{}, {}, num_qubits});
	State& init_state = state_stack.back();
	init_state.sel_cols.resize(matrix.num_columns());
	init_state.rem_rows.resize(matrix.num_rows());
	std::iota(init_state.sel_cols.begin(), init_state.sel_cols.end(), 0u);
	std::iota(init_state.rem_rows.begin(), init_state.rem_rows.end(), 0u);

	while (!state_stack.empty()) {
		State state = std::move(state_stack.back());
		state_stack.pop_back();
		if (state.qubit != num_qubits) {
			add_gate(state, matrix, gates);
		}

		auto temp = std::valarray(matrix.column(state.sel_cols.back()));
		if (state.sel_cols.size() == 1 && temp.sum() <= 1) {
			continue;
		}
		if (state.rem_rows.empty()) {
			continue;
		}

		uint32_t sel_row = select_row(state, matrix);
		std::vector<uint32_t> cofactor0;
		std::vector<uint32_t> cofactor1;
		for (uint32_t col : state.sel_cols) {
			if (matrix(sel_row, col)) {
				cofactor1.push_back(col);
				continue;
			}
			cofactor0.push_back(col);
		}
		std::remove(
		    state.rem_rows.begin(), state.rem_rows.end(), sel_row);
		state.rem_rows.pop_back();
		if (!cofactor1.empty()) {
			state_stack.emplace_back(std::move(cofactor1),
			    state.rem_rows,
			    (state.qubit == num_qubits) ? sel_row : state.qubit);
		}
		if (!cofactor0.empty()) {
			state_stack.emplace_back(
			    std::move(cofactor0), state.rem_rows, state.qubit);
		}
	}
	return gates;
}

} // namespace gray_synth_detail

// Each column is a parity, num_rows = num_qubits
inline void gray_synth(Circuit& circuit, std::vector<WireRef> const& qubits)
{
	Matrix<uint8_t> matrix = {{0, 1, 1, 1, 1, 1}, {1, 0, 0, 1, 1, 1},
	    {1, 0, 0, 1, 0, 0}, {0, 0, 1, 0, 1, 0}};

	auto gates = gray_synth_detail::synthesize(qubits, matrix);
	for (auto const& [control, target] : gates) {
		circuit.create_instruction(
		    GateLib::X(), {qubits[control]}, qubits[target]);
	}

	Matrix<uint8_t> transform(qubits.size(), qubits.size());
	for (uint32_t i = 0; i < qubits.size(); ++i) {
		transform(i, i) = 1;
	}
	std::reverse(gates.begin(), gates.end());
	for (auto const& [control, target] : gates) {
		transform.row(target) ^= std::valarray(transform.row(control));
	}
	cnot_synth(circuit, qubits, transform);
}

inline Circuit gray_synth(uint32_t num_qubits)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");

	// Create the necessary qubits
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	gray_synth(circuit, wires);
	return circuit;
}

} // namespace tweedledum
