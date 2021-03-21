/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/steiner_gauss_synth.h"
#include "tweedledum/Operators/Standard/X.h"

namespace tweedledum {

namespace {
using AbstractGate = std::pair<uint32_t, uint32_t>;

inline void synthesize(Circuit& circuit, Device const& device, BMatrix matrix)
{
    constexpr auto inf = std::numeric_limits<uint32_t>::max();
    std::vector<AbstractGate> gates;
    std::vector<uint8_t> above_diagonal(matrix.size(), 0);
    for (auto col = 0u; col < matrix.cols(); ++col) {
        // Find pivot
        uint32_t pivot = inf;
        bool crossed_diag = false;
        std::vector<AbstractGate> swap;
        std::vector<AbstractGate> cleanup_swap;
        if (matrix(col, col) == MyBool(1u)) {
            pivot = col;
        } else {
            // If the diagonal is not one, we need to look for a pivot, i.e, a
            // row with an one, and move it to the diagonal.  Of course that we 
            // just don't take any pivot, we look for the one closer to the 
            // diagonal.
            uint32_t min_dist = inf;
            for (auto row = col + 1; row < matrix.rows(); ++row){
                if (matrix(row, col) == MyBool(0u)) {
                    continue;
                }
                if (device.distance(row, col) < min_dist) {
                    pivot = row;
                    min_dist = device.distance(row, col);
                }
            }
            assert(pivot != inf);
            auto path = device.shortest_path(pivot, col);
            uint32_t control = pivot;
            for (uint32_t i = 1; i < path.size(); ++i) {
                uint32_t const target = path.at(i);
                if (matrix(target, col) == MyBool(1u)) {
                    continue;
                }
                matrix.row(target) += matrix.row(control);
                swap.emplace_back(control, target);
                if (control < col) {
                    crossed_diag = true;
                }
                above_diagonal.at(target) |= above_diagonal.at(control) || crossed_diag;
                control = target;
            }
            if (crossed_diag) {
                uint32_t target = col;
                for (auto it = std::next(path.rbegin()); it != path.rend(); it++) {
                    uint32_t control = *it;
                    if (target == col) {
                        continue;
                    }
                    matrix.row(target) += matrix.row(control);
                    swap.emplace_back(control, target);
                    above_diagonal.at(target) |= above_diagonal.at(control) | (control < col);
                    target = control;
                }
                for (auto it = swap.rbegin(); it != swap.rend(); it++) {
                    auto const& [control, target] = *it;
                    if (above_diagonal.at(target) && control != pivot) {
                        matrix.row(target) += matrix.row(control);
                        cleanup_swap.emplace_back(control, target);
                    }
                }
            }
        }

        // Compute steiner tree covering the 1's in column i
        pivot = col;
        std::fill(above_diagonal.begin(), above_diagonal.end(), 0);
        std::vector<uint32_t> terminals;
        for (uint32_t row = 0; row < matrix.rows(); ++row) {
            if (row == col || matrix(row, col) == MyBool(0u)) {
                continue;
            }
            terminals.push_back(row);
        }
        if (terminals.empty()) {
            continue;
        }
        auto s_tree = device.steiner_tree(terminals, pivot);

        // Propagate 1's to column col for each Steiner point
        std::vector<AbstractGate> compute;
        for (auto const& [control, target] : s_tree) {
            if (matrix(target, col) == MyBool(1u)) {
                continue;
            }
            matrix.row(target) += matrix.row(control);
            compute.emplace_back(control, target);
            above_diagonal.at(target) |= above_diagonal.at(control) | (control < pivot);
        }

        // Empty all 1's from column i in the Steiner tree
        for (auto it = s_tree.rbegin(); it != s_tree.rend(); it++) {
            auto const [control, target] = *it;
            matrix.row(target) += matrix.row(control);
            compute.emplace_back(control, target); 
            above_diagonal.at(target) |= above_diagonal.at(control) | (control < pivot);
        }

        // For each node that has an above diagonal dependency,
        // reverse the previous steps to undo the additions
        std::vector<AbstractGate> cleanup;
        for (auto it = compute.rbegin(); it != compute.rend(); it++) {
            auto const [control, target] = *it;
            if (above_diagonal.at(target) && control != pivot) {
                matrix.row(target) += matrix.row(control);
                cleanup.emplace_back(control, target);
            }
        }

        std::copy(swap.begin(), swap.end(), std::back_inserter(gates));
        std::copy(cleanup_swap.begin(), cleanup_swap.end(), std::back_inserter(gates));
        std::copy(compute.begin(), compute.end(), std::back_inserter(gates));
        std::copy(cleanup.begin(), cleanup.end(), std::back_inserter(gates));
    }
    std::reverse(gates.begin(), gates.end());
    for (auto const& [c, t] : gates) {
        auto const control = circuit.qubit(c);
        auto const target = circuit.qubit(t);
        circuit.apply_operator(Op::X(), {control, target});
    }
}

}

void steiner_gauss_synth(Circuit& circuit, Device const& device,
    BMatrix const& matrix, nlohmann::json const& config)
{
    assert(matrix.rows() == matrix.cols());
    assert(matrix.rows() == circuit.num_qubits());
    assert(circuit.num_qubits() == device.num_qubits());
    synthesize(circuit, device, matrix);
}

Circuit steiner_gauss_synth(Device const& device, BMatrix const& matrix,
    nlohmann::json const& config)
{
    Circuit circuit;
    uint32_t const num_qubits = device.num_qubits();
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        circuit.create_qubit();
    }
    steiner_gauss_synth(circuit, device, matrix, config);
    return circuit;
}

} // namespace tweedledum