/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/LinearPP.h"
#include "../../support/Matrix.h"
#include "cnot_synth.h"

// This implementation is based on:
//
// Amy, Matthew, Parsiad Azimzadeh, and Michele Mosca. "On the controlled-NOT
// complexity of controlled-NOT–phase circuits." Quantum Science and Technology
// 4.1 (2018): 015002.
//
// This synthesis method generates a CNOT-dihedral circuit.  In principle it 
// serves the same purpose of 'all_linear_synth' algorithm, but with two 
// important differences:
//     (1) it will __not__ necessarily generate all possible linear 
//         combinations.  Thus, the synthesized circuit can potentially be of
//         better.
//     (2) The overall linear transformation can be defined
// quality (in the number of gates).
//
// __NOTE__: if you indeed require all linear combinations, then 
// all_linear_synth __will be faster__.
//
namespace tweedledum {
#pragma region Implementation details
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
#pragma endregion

/*! \brief Synthesis of a CNOT-dihedral circuits with all linear combinations.
 *
 * This is the in-place variant of ``all_linear_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 * 
 * \param[inout] circuit A circuit in which the parities will be synthesized on.
 * \param[in] qubits The qubits that will be used.
 * \param[in] linear_trans The overall linear transformation
 * \param[in] parities List of parities and their associated angles.
 */
// Each column is a parity, num_rows = num_qubits
template<typename T, typename Parity>
inline void gray_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    Matrix<T> linear_trans, LinearPP<Parity> parities)
{
	// FIXME: This part assumes that Parity is a bit string implemented 
	// using an integer type such as uint32_t or uint64_t, or a
	// a DynamicBitset.
	Matrix<uint8_t> parities_matrix(qubits.size(), parities.size());
	uint32_t col = 0;
	for (auto const& [parity, angle] : parities) {
		for (uint32_t row = 0; row < qubits.size(); ++row) {
			parities_matrix(row, col) = (parity >> row) & 1;
		}
		++col;
	}

	auto gates = gray_synth_detail::synthesize(qubits, parities_matrix);
	// Initialize the parity of each qubit state
	// Applying phase gate to parities that consisting of just one variable
	// i is the index of the target
	std::vector<uint32_t> qubits_states(circuit.num_qubits(), 0);
	for (uint32_t i = 0u; i < circuit.num_qubits(); ++i) {
		qubits_states[i] = (1u << i);
		auto angle = parities.extract_term(qubits_states[i]);
		if (angle != 0.0) {
			circuit.create_instruction(
			    GateLib::R1(angle), {qubits[i]});
		}
	}
	// Effectively create the circuit
	for (auto const& [control, target] : gates) {
		circuit.create_instruction(
		    GateLib::X(), {qubits[control]}, qubits[target]);
		qubits_states[target] ^= qubits_states[control];
		linear_trans.row(target) ^= std::valarray(linear_trans.row(control));
		auto angle = parities.extract_term(qubits_states[target]);
		if (angle != 0.0) {
			circuit.create_instruction(
			    GateLib::R1(angle), {qubits[target]});
		}
	}
	// Synthesize the overall linear transformation
	cnot_synth(circuit, qubits, linear_trans);
}

/*! \brief Synthesis of a CNOT-dihedral circuits.
 *
 * \param[in] num_qubits The number of qubits.
 * \param[in] parities List of parities and their associated angles.
 * \return A CNOT-dihedral circuit on `num_qubits`.
 */
template<typename Parity>
inline Circuit gray_synth(uint32_t num_qubits, LinearPP<Parity> const& parities)
{
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");

	// Create the necessary qubits
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}

	// Create the linear
	Matrix<uint8_t> linear_trans(num_qubits, num_qubits);
	for (uint32_t i = 0; i < num_qubits; ++i) {
		linear_trans(i, i) = 1;
	}
	gray_synth(circuit, wires, linear_trans, parities);
	return circuit;
}

} // namespace tweedledum
