/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Synthesis/linear_synth.h"
#include "tweedledum/Operators/Standard.h"

namespace tweedledum {

struct params {
    bool best_effort;
    uint32_t section_size;
};

using AbstractGate = std::pair<uint32_t, uint32_t>;
using GateList = std::vector<AbstractGate>;

inline void pattern_elimination(Matrix& matrix, uint32_t start, uint32_t end, GateList& gates)
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

inline void gaussian_elimination(Matrix& matrix, uint32_t start, uint32_t end, GateList& gates)
{
    for (uint32_t col = start; col < end; ++col) {
        bool is_diagonal_one = (matrix(col, col) == 1);
        for (uint32_t row = col + 1; row < matrix.num_rows(); ++row) {
            if (matrix(row, col) == 0) {
                continue;
            }
            if (!is_diagonal_one) {
                is_diagonal_one = 1;
                matrix.row(col) ^= std::valarray(matrix.row(row));
                gates.emplace_back(row, col);
            }
            matrix.row(row) ^= std::valarray(matrix.row(col));
            gates.emplace_back(col, row);
        }
    }
}

inline GateList lower_cnot_synthesis(Matrix& matrix, uint32_t section_size)
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

void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits,
    Matrix matrix, uint32_t const section_size)
{
    GateList lower = lower_cnot_synthesis(matrix, section_size);
    // TODO: Implement a transpose inpace
    auto transposed = transpose(matrix);
    GateList upper = lower_cnot_synthesis(transposed, section_size);

    for (auto const& [control, target] : upper) {
        // switch control/target of CX gates in gates_upper;
        circuit.apply_operator(Op::X(), {qubits[target], qubits[control]});
    }
    std::reverse(lower.begin(), lower.end());
    for (auto const& [control, target] : lower) {
        circuit.apply_operator(Op::X(), {qubits[control], qubits[target]});
    }
}

void best_effort_synthesize(
    Circuit& circuit, std::vector<WireRef> const& qubits, Matrix const& matrix)
{
    Matrix temp_matrix = matrix;
    GateList lower = lower_cnot_synthesis(temp_matrix, 1u);
    auto transposed = transpose(temp_matrix);
    GateList upper = lower_cnot_synthesis(transposed, 1u);
    auto best_size = lower.size() + upper.size();

    // Minimum/Maximum section sizes (ss)
    uint32_t const min_ss = 2u;
    uint32_t const max_ss = matrix.num_columns();
    uint32_t best_ss = min_ss;
    uint32_t current_ss = min_ss;
    // fmt::print("[{}]\n", qubits.size());
    do {
        Matrix temp_matrix = matrix;
        GateList lower = lower_cnot_synthesis(temp_matrix, current_ss);
        auto transposed = transpose(temp_matrix);
        GateList upper = lower_cnot_synthesis(transposed, current_ss);
        uint32_t const size = lower.size() + upper.size();
        if (size < best_size) {
            best_size = size;
            best_ss = current_ss;
        }
        uint32_t const temp = std::ceil(best_size * 1.1);
        if (size > temp) {
            break;
        }
        ++current_ss;
    } while ((current_ss < max_ss) && (best_size > 1));
    synthesize(circuit, qubits, matrix, best_ss);
}

void linear_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    Matrix const& matrix, nlohmann::json const& config)
{
    // double k = std::floor(std::log2(double(qubits.size())) / 2.0);
    params params = {false, 2};
    auto cnot_cfg = config.find("linear_synth");
    if (cnot_cfg != config.end()) {
        if (cnot_cfg->contains("best_effort")) {
            params.best_effort = cnot_cfg->at("best_effort");
        }
        if (cnot_cfg->contains("section_size")) {
            params.section_size = cnot_cfg->at("section_size");
        }
    }
    if (params.best_effort) {
        best_effort_synthesize(circuit, qubits, matrix);
        return;
    }
    synthesize(circuit, qubits, matrix, params.section_size);
}

Circuit linear_synth(Matrix const& matrix, nlohmann::json const& config)
{
    assert(matrix.num_rows() == matrix.num_columns());
    Circuit circuit;

    // Create the necessary qubits
    uint32_t const num_qubits = matrix.num_rows();
    std::vector<WireRef> wires;
    wires.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        wires.emplace_back(circuit.create_qubit());
    }
    linear_synth(circuit, wires, matrix, config);
    return circuit;
}

} // namespace tweedledum