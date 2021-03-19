/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Synthesis/gray_synth.h"
#include "tweedledum/Synthesis/linear_synth.h"

#include <vector>

namespace tweedledum {

namespace {
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

inline uint32_t select_row(State const& state, BMatrix const& matrix)
{
    assert(!state.rem_rows.empty());
    uint32_t sel_row = 0;
    uint32_t max = 0;

    for (uint32_t row_idx : state.rem_rows) {
        auto const& row = matrix.row(row_idx).array();
        uint32_t const num_ones = (row == MyBool(1)).count();
        uint32_t const num_zeros = matrix.cols() - num_ones;
        uint32_t const local_max = std::max(num_ones, num_zeros);
        if (local_max > max) {
            max = local_max;
            sel_row = row_idx;
        }
    }
    return sel_row;
}

inline void add_gate(State const& state, BMatrix& matrix, GateList& gates)
{
    for (uint32_t j = 0u; j < matrix.rows(); ++j) {
        if (j == state.qubit) {
            continue;
        }
        bool all_one = true;
        for (uint32_t col : state.sel_cols) {
            all_one &= (matrix(j, col) == MyBool(1));
        }
        if (!all_one) {
            continue;
        }
        matrix.row(j) += matrix.row(state.qubit);
        gates.emplace_back(j, state.qubit);
    }
}

inline GateList synthesize(std::vector<Qubit> const& qubits, BMatrix& matrix)
{
    GateList gates;
    uint32_t const num_qubits = qubits.size();
    // Initial state
    std::vector<State> state_stack(1, {{}, {}, num_qubits});
    State& init_state = state_stack.back();
    init_state.sel_cols.resize(matrix.cols(), 0);
    init_state.rem_rows.resize(matrix.rows(), 0);
    std::iota(init_state.sel_cols.begin(), init_state.sel_cols.end(), 0u);
    std::iota(init_state.rem_rows.begin(), init_state.rem_rows.end(), 0u);

    while (!state_stack.empty()) {
        State state = std::move(state_stack.back());
        state_stack.pop_back();
        if (state.qubit != num_qubits) {
            add_gate(state, matrix, gates);
        }
        assert(!state.sel_cols.empty());
        auto const& temp = matrix.col(state.sel_cols.back()).array();
        uint32_t const ones = (temp == MyBool(1)).count();
        if (state.sel_cols.size() == 1 && ones <= 1) {
            continue;
        }
        if (state.rem_rows.empty()) {
            continue;
        }

        uint32_t sel_row = select_row(state, matrix);
        std::vector<uint32_t> cofactor0;
        std::vector<uint32_t> cofactor1;
        for (uint32_t col : state.sel_cols) {
            if (matrix(sel_row, col) == MyBool(1)) {
                cofactor1.push_back(col);
                continue;
            }
            cofactor0.push_back(col);
        }
        std::remove(state.rem_rows.begin(), state.rem_rows.end(), sel_row);
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
}

void gray_synth(Circuit& circuit, std::vector<Qubit> const& qubits, 
    std::vector<Cbit> const& cbits, BMatrix linear_trans, LinPhasePoly phase_parities,
    nlohmann::json const& config)
{
    if (phase_parities.size() == 0) {
        return;
    }
    BMatrix parities_matrix = BMatrix::Zero(qubits.size(), phase_parities.size());
    uint32_t col = 0;
    for (auto const& [parity, angle] : phase_parities) {
        for (uint32_t lit : parity) {
            parities_matrix((lit >> 1) - 1, col) = MyBool(1);
        }
        ++col;
    }

    auto gates = synthesize(qubits, parities_matrix);
    // Initialize the parity of each qubit state
    // Applying phase gate to phase_parities that consisting of just one variable
    // i is the index of the target
    std::vector<uint32_t> qubits_states(circuit.num_qubits(), 0);
    for (uint32_t i = 0u; i < circuit.num_qubits(); ++i) {
        qubits_states.at(i) = (1u << i);
        auto angle = phase_parities.extract_phase(qubits_states.at(i));
        if (angle != 0.0) {
            circuit.apply_operator(Op::P(angle), {qubits.at(i)}, cbits);
        }
    }
    // Effectively create the circuit
    for (auto const& [control, target] : gates) {
        circuit.apply_operator(Op::X(), {qubits.at(control), qubits.at(target)},
            cbits);
        qubits_states.at(target) ^= qubits_states.at(control);
        linear_trans.row(target) += linear_trans.row(control);
        auto angle = phase_parities.extract_phase(qubits_states.at(target));
        if (angle != 0.0) {
            circuit.apply_operator(Op::P(angle), {qubits.at(target)}, cbits);
        }
    }

    // Synthesize the overall linear transformation
    nlohmann::json linear_synth_cfg;
    if (config.contains("linear_synth")) {
        linear_synth_cfg["linear_synth"] = config["linear_synth"];
    }
    linear_synth_cfg["linear_synth"]["inverse"] = true;
    linear_synth(circuit, qubits, cbits, linear_trans, linear_synth_cfg);
}

Circuit gray_synth(uint32_t num_qubits, LinPhasePoly const& phase_parities,
    nlohmann::json const& config)
{
    Circuit circuit;
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    BMatrix linear_trans = BMatrix::Identity(num_qubits, num_qubits);
    gray_synth(circuit, qubits, {}, linear_trans, phase_parities, config);
    return circuit;
}

} // namespace tweedledum
