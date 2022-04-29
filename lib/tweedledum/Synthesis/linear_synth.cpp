/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/linear_synth.h"
#include "tweedledum/Operators/Standard.h"

namespace tweedledum {

namespace {
struct Config {
    uint32_t section_size;
    bool best_effort;
    bool inverse;

    Config(nlohmann::json const& config)
        : section_size(2u)
        , best_effort(false)
        , inverse(false)
    {
        auto cfg = config.find("linear_synth");
        if (cfg != config.end()) {
            if (cfg->contains("section_size")) {
                section_size = cfg->at("section_size");
            }
            if (cfg->contains("best_effort")) {
                best_effort = cfg->at("best_effort");
            }
            if (cfg->contains("inverse")) {
                inverse = cfg->at("inverse");
            }
        }
    }
};

using AbstractGate = std::pair<uint32_t, uint32_t>;
using GateList = std::vector<AbstractGate>;

inline void pattern_elimination(
  BMatrix& matrix, uint32_t start, uint32_t end, GateList& gates)
{
    std::vector<uint32_t> table(matrix.rows(), 0);
    auto const begin = table.begin() + start;
    for (uint32_t row = start; row < matrix.rows(); ++row) {
        uint32_t pattern = 0u;
        uint32_t subrow_col = 0u;
        for (uint32_t col = start; col < end; ++col) {
            pattern |= (static_cast<uint32_t>(matrix(row, col)) << subrow_col);
            subrow_col += 1;
        }
        if (pattern == 0) {
            continue;
        }
        auto const it = std::find(begin, table.end(), pattern);
        if (it != table.end()) {
            uint32_t pos = std::distance(table.begin(), it);
            matrix.row(row) += matrix.row(pos); // ^=
            gates.emplace_back(pos, row);
        } else {
            table.at(row) = pattern;
        }
    }
}

inline void gaussian_elimination(
  BMatrix& matrix, uint32_t start, uint32_t end, GateList& gates)
{
    for (uint32_t col = start; col < end; ++col) {
        bool is_diagonal_one = (matrix(col, col) == MyBool(1u));
        for (uint32_t row = col + 1; row < matrix.rows(); ++row) {
            if (matrix(row, col) == MyBool(0u)) {
                continue;
            }
            if (!is_diagonal_one) {
                is_diagonal_one = 1;
                matrix.row(col) += matrix.row(row);
                gates.emplace_back(row, col);
            }
            matrix.row(row) += matrix.row(col);
            gates.emplace_back(col, row);
        }
    }
}

inline GateList lower_cnot_synthesis(BMatrix& matrix, uint32_t section_size)
{
    GateList gates;
    uint32_t const num_cols = matrix.cols();
    uint32_t const num_sections = (num_cols - 1u) / section_size + 1u;
    for (uint32_t section = 0u; section < num_sections; ++section) {
        uint32_t start = section * section_size;
        uint32_t end = std::min(start + section_size, num_cols);
        pattern_elimination(matrix, start, end, gates);
        gaussian_elimination(matrix, start, end, gates);
    }
    return gates;
}

inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, BMatrix matrix, uint32_t const section_size,
  bool const inverse)
{
    GateList lower = lower_cnot_synthesis(matrix, section_size);
    matrix.transposeInPlace();
    GateList upper = lower_cnot_synthesis(matrix, section_size);
    assert(matrix.isIdentity());

    GateList to_add;
    to_add.reserve(lower.size() + upper.size());

    // In the paper the authors adde gates to the begining of the gate list.
    // Here, I do add to the back, then instead of reversing upper, I reverse
    // lower.  I still need to switch control/target in upper!
    std::for_each(upper.begin(), upper.end(),
      [&](auto const& gate) { to_add.emplace_back(gate.second, gate.first); });

    std::for_each(lower.rbegin(), lower.rend(),
      [&](auto const& gate) { to_add.emplace_back(gate); });

    if (inverse) {
        std::reverse(to_add.begin(), to_add.end());
    }
    for (auto const& [control, target] : to_add) {
        circuit.apply_operator(
          Op::X(), {qubits[control], qubits[target]}, cbits);
    }
}

inline void best_effort_synthesize(Circuit& circuit,
  std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits,
  BMatrix const& matrix, bool const reverse)
{
    BMatrix temp_matrix = matrix;
    GateList lower = lower_cnot_synthesis(temp_matrix, 1u);
    temp_matrix.transposeInPlace();
    GateList upper = lower_cnot_synthesis(temp_matrix, 1u);
    auto best_size = lower.size() + upper.size();

    // Minimum/Maximum section sizes (ss)
    uint32_t const min_ss = 2u;
    uint32_t const max_ss = matrix.cols();
    uint32_t best_ss = min_ss;
    uint32_t current_ss = min_ss;
    do {
        BMatrix temp_matrix = matrix;
        GateList lower = lower_cnot_synthesis(temp_matrix, 1u);
        temp_matrix.transposeInPlace();
        GateList upper = lower_cnot_synthesis(temp_matrix, 1u);
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
    synthesize(circuit, qubits, cbits, matrix, best_ss, reverse);
}
} // namespace

void linear_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, BMatrix const& matrix,
  nlohmann::json const& config)
{
    Config cfg(config);
    if (cfg.best_effort) {
        best_effort_synthesize(circuit, qubits, cbits, matrix, cfg.inverse);
        return;
    }
    synthesize(circuit, qubits, cbits, matrix, cfg.section_size, cfg.inverse);
}

Circuit linear_synth(BMatrix const& matrix, nlohmann::json const& config)
{
    assert(matrix.rows() == matrix.cols());
    Circuit circuit;
    uint32_t const num_qubits = matrix.rows();
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    linear_synth(circuit, qubits, {}, matrix, config);
    return circuit;
}

} // namespace tweedledum