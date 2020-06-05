/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"
#include "../../support/Matrix.h"

namespace tweedledum {
namespace cnot_synth_detail {

using AbstractGate = std::pair<uint32_t, uint32_t>;
using GateList = std::vector<AbstractGate>;

template<typename T>
inline void pattern_elimination(
    Matrix<T>& matrix, uint32_t start, uint32_t end, GateList& gates)
{
	std::vector<uint32_t> table(matrix.num_rows(), 0);
	auto const begin = table.begin() + start;
	for (uint32_t row = start; row < matrix.num_rows(); ++row) {
		uint32_t pattern = 0u;
		uint32_t subrow_col = 0u;
		for (uint32_t col = start; col < end; ++col) {
			pattern |= (matrix(row, col) << subrow_col);
			subrow_col += 1;
		}
		if (pattern == 0) {
			continue;
		}
		auto const it = std::find(begin, table.end(), pattern);
		if (it != table.end()) {
			uint32_t pos = std::distance(table.begin(), it);
			matrix.row(row) ^= std::valarray(matrix.row(pos));
			gates.emplace_back(pos, row);
		} else {
			table[row] = pattern;
		}
	}
}

template<typename T>
inline void gaussian_elimination(
    Matrix<T>& matrix, uint32_t start, uint32_t end, GateList& gates)
{
	for (uint32_t col = start; col < end; ++col) {
		bool is_diagonal_one = (matrix(col, col) == 1);
		for (uint32_t row = col + 1; row < matrix.num_rows(); ++row) {
			if (matrix(row, col) == 0) {
				continue;
			}
			if (!is_diagonal_one) {
				is_diagonal_one = 1;
				matrix.row(col)
				    ^= std::valarray(matrix.row(row));
				gates.emplace_back(row, col);
			}
			matrix.row(row) ^= std::valarray(matrix.row(col));
			gates.emplace_back(col, row);
		}
	}
}

template<typename T>
inline GateList lower_cnot_synthesis(Matrix<T>& matrix, uint32_t section_size)
{
	GateList gates;
	uint32_t const num_cols = matrix.num_columns();
	uint32_t const num_sections = (num_cols - 1u) / section_size + 1u;
	for (uint32_t section = 0u; section < num_sections; ++section) {
		uint32_t start = section * section_size;
		uint32_t end = std::min(start + section_size, num_cols);
		pattern_elimination(matrix, start, end, gates);
		gaussian_elimination(matrix, start, end, gates);
	}
	return gates;
}

template<typename T>
void synthesize(
    Circuit& circuit, std::vector<WireRef> const& qubits, Matrix<T> matrix)
{
	GateList lower = lower_cnot_synthesis(matrix, 2u);
	// TODO: Implement a transpose inpace
	auto transposed = transpose(matrix);
	GateList upper = lower_cnot_synthesis(transposed, 2u);

	for (auto const& [control, target] : upper) {
		// switch control/target of CX gates in gates_upper;
		circuit.create_instruction(
		    GateLib::X(), {qubits[target]}, qubits[control]);
	}
	std::reverse(lower.begin(), lower.end());
	for (auto const& [control, target] : lower) {
		circuit.create_instruction(
		    GateLib::X(), {qubits[control]}, qubits[target]);
	}
}

} // namespace cnot_synth_detail

template<typename T>
inline void cnot_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    Matrix<T> const& matrix)
{
	cnot_synth_detail::synthesize(circuit, qubits, matrix);
}

template<typename T>
inline Circuit cnot_synth(Matrix<T> const& matrix)
{
	assert(matrix.num_rows() == matrix.num_columns());
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");

	// Create the necessary qubits
	uint32_t const num_qubits = matrix.num_rows();
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	cnot_synth(circuit, wires, matrix);
	return circuit;
}

} // namespace tweedledum
