/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/decomp_synth.h"
#include "tweedledum/Operators/Reversible.h"
#ifdef _MSC_VER
    #include "tweedledum/Utils/Intrinsics.h"
#endif

#include <kitty/kitty.hpp>
#include <list>

// This implementation is based on:
//
// De Vos, Alexis, and Yvan Van Rentergem. "Young subgroups for reversible
// computers." Advances in Mathematics of Communications 2.2 (2008): 183.
//
// In decomposition-based synthesis the reversible function is recursively
// decomposed into simpler functions based on the Young subgroup decomposition:
//
//     Given a wire Wi, every reversible function f can be decomposed into three
//     functions f = g1 o f' o g2, where g1 and g2 can be realized with a
//     truth table gate on Wi and f' is a reversible function that does not
//     change in Wi.
//
// Based on this decomposition, this synthesis algorithms determine the gates
// for g1 and g2 and then recur on f'.
//
namespace tweedledum {

namespace {

inline auto decompose(std::vector<uint32_t>& perm, uint32_t var)
{
    std::vector<uint32_t> left(perm.size(), 0);
    std::vector<uint32_t> right(perm.size(), 0);
    std::vector<uint8_t> visited(perm.size(), 0);

    uint32_t row = 0u;
    do {
        // Assign 0 to var on left side
        left[row] = (row & ~(1 << var));
        visited[row] = 1;
        // Assign 1 to var on left side
        left[row ^ (1 << var)] = left[row] ^ (1 << var);
        row ^= (1 << var);
        visited[row] = 1;

        // Assign 1 to var on right side
        right[perm[row] | (1 << var)] = perm[row];
        // Assign 0 to var on right side
        right[perm[row] & ~(1 << var)] = perm[row] ^ (1 << var);

        // Find next row
        uint32_t i = 0;
        for (; i < perm.size() - 1; ++i) {
            if (perm[i] == (perm[row] ^ (1 << var))) {
                break;
            }
        }
        // If this rows was already visited, find a new one.
        if (visited[i]) {
            for (i = 0; i < perm.size(); ++i) {
                if (visited[i] == 0) {
                    break;
                }
            }
            // If there is no unvisited rows, we are done!
            if (i == perm.size()) {
                break;
            }
        }
        row = i;
    } while (true);

    std::vector<uint32_t> perm_old = perm;
    for (uint32_t i = 0; i < perm.size(); ++i) {
        perm[left[i]] = right[perm_old[i]];
    }
    return std::make_pair(std::move(left), std::move(right));
}

inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, std::vector<uint32_t> perm)
{
    std::vector<kitty::dynamic_truth_table> truth_tables;
    truth_tables.reserve(qubits.size() * 2);
    std::list<std::pair<uint32_t, std::vector<Qubit>>> gates;
    auto pos = gates.begin();
    for (uint32_t i = 0u; i < qubits.size(); ++i) {
        auto&& [left, right] = decompose(perm, i);
        auto& left_tt = truth_tables.emplace_back(qubits.size());
        auto& right_tt = truth_tables.emplace_back(qubits.size());
        for (uint32_t row = 0; row < perm.size(); ++row) {
            if (left[row] != row) {
                kitty::set_bit(left_tt, row);
            }
            if (right[row] != row) {
                kitty::set_bit(right_tt, row);
            }
        }

        if (!kitty::is_const0(left_tt)) {
            std::vector<Qubit> left_qubits;
            for (auto element : kitty::min_base_inplace(left_tt)) {
                left_qubits.emplace_back(qubits.at(element));
            }
            left_tt = kitty::shrink_to(left_tt, left_qubits.size());
            left_qubits.push_back(qubits.at(i));
            pos = gates.emplace(
              pos, truth_tables.size() - 2, std::move(left_qubits));
            ++pos;
        }
        if (!kitty::is_const0(right_tt)) {
            std::vector<Qubit> right_qubits;
            for (auto element : kitty::min_base_inplace(right_tt)) {
                right_qubits.emplace_back(qubits.at(element));
            }
            right_tt = kitty::shrink_to(right_tt, right_qubits.size());
            right_qubits.push_back(qubits.at(i));
            pos = gates.emplace(
              pos, truth_tables.size() - 1, std::move(right_qubits));
        }
    }
    for (auto const& [tt_idx, qubits] : gates) {
        circuit.apply_operator(
          Op::TruthTable(truth_tables.at(tt_idx)), qubits, cbits);
    }
}

} // namespace

void decomp_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, std::vector<uint32_t> const& perm)
{
    assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
    synthesize(circuit, qubits, cbits, perm);
}

Circuit decomp_synth(std::vector<uint32_t> const& perm)
{
    assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
    Circuit circuit;
    uint32_t const num_qubits = __builtin_ctz(perm.size());
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    decomp_synth(circuit, qubits, {}, perm);
    return circuit;
}

} // namespace tweedledum
