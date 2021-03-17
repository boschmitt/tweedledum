/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/transform_synth.h"

#include "tweedledum/Operators/Standard.h"
#ifdef _MSC_VER
#include "tweedledum/Utils/Intrinsics.h"
#endif

namespace tweedledum {

namespace {
using AbstractGate = std::pair<uint32_t, uint32_t>;
using GateList = std::vector<AbstractGate>;

inline void update_permutation(
    std::vector<uint32_t>& perm, uint32_t controls, uint32_t targets)
{
    for (uint32_t i = 0; i < perm.size(); ++i) {
        if ((perm[i] & controls) == controls) {
            perm[i] ^= targets;
        }
    }
}

inline GateList unidirectional(std::vector<uint32_t> perm)
{
    GateList gates;
    for (uint32_t i = 0u; i < perm.size(); ++i) {
        // Skip identity lines
        if (perm[i] == i) {
            continue;
        }
        uint32_t const y = perm[i];
        // Let p be the bit string with 1's in all positions where the
        // binary expansion of i is 1 while the expansion of perm[i] is
        // 0.
        uint32_t const p = i & ~y;
        // For each pj = 1, apply the Toffoli gate with control lines
        // corresponding to all outputs in positions where the expansion
        // of a is 1 and whose target line is the output in position j.
        if (p) {
            update_permutation(perm, y, p);
            gates.emplace_back(y, p);
        }

        // Lett q be the bit string with 1's in all positions where the
        // expansion of i is 0 while the expansion of perm[i] is 1.
        uint32_t const q = ~i & y;
        // For each qk = 1, apply the Toffoli gate with control lines
        // corresponding to all outputs in positions where the expansion
        // of f+(i) is 1 and whose target line is the output in k.
        if (q) {
            update_permutation(perm, i, q);
            gates.emplace_back(i, q);
        }
    }
    std::reverse(gates.begin(), gates.end());
    return gates;
}

inline void update_permutation_inv(
    std::vector<uint32_t>& perm, uint32_t controls, uint32_t targets)
{
    for (uint32_t i = 0u; i < perm.size(); ++i) {
        if ((i & controls) != controls) {
            continue;
        }
        uint32_t const partner = i ^ targets;
        if ( partner > i) {
            std::swap(perm[i], perm[partner]);
        }
    }
}

inline GateList bidirectional(std::vector<uint32_t> perm)
{
    GateList gates;
    auto pos = gates.begin();
    for (uint32_t i = 0u; i < perm.size(); ++i) {
        if (perm[i] == i) {
            continue;
        }
        uint32_t const y = perm[i];
        uint32_t const x = std::distance(
            perm.begin(), std::find(perm.begin() + i, perm.end(), i));

        if (__builtin_popcount(i ^ y) <= __builtin_popcount(i ^ x)) {
            uint32_t const p = i & ~y;
            if (p) {
                update_permutation(perm, y, p);
                pos = gates.emplace(pos, y, p);
            }
            uint32_t const q = ~i & y;
            if (q) {
                update_permutation(perm, i, q);
                pos = gates.emplace(pos, i, q);
            }
            continue;
        }
        uint32_t const p = ~x & i;
        if (p) {
            update_permutation_inv(perm, x, p);
            pos = gates.emplace(pos, x, p);
            ++pos;
        }
        uint32_t const q = x & ~i;
        if (q) {
            update_permutation_inv(perm, i, q);
            pos = gates.emplace(pos, i, q);
            ++pos;
        }
    }
    return gates;
}

inline GateList multidirectional(std::vector<uint32_t> perm)
{
    GateList gates;
    auto pos = gates.begin();
    for (uint32_t i = 0u; i < perm.size(); ++i) {
        // Find cheapest assignment
        uint32_t x_best = i;
        uint32_t x_best_cost = __builtin_popcount(i ^ perm[i]);
        for (uint32_t j = i + 1; j < perm.size(); ++j) {
            uint32_t j_cost = __builtin_popcount(i ^ perm[j]);
            uint32_t cost = __builtin_popcount(i ^ j) + j_cost;
            if (cost < x_best_cost) {
                x_best = j;
                x_best_cost = cost;
            }
        }

        uint32_t const y = perm[x_best];
        // map x |-> i
        uint32_t p = ~x_best & i;
        if (p) {
            update_permutation_inv(perm, x_best, p);
            pos = gates.emplace(pos, x_best, p);
            pos++;
        }
        uint32_t q = x_best & ~i;
        if (q) {
            update_permutation_inv(perm, i, q);
            pos = gates.emplace(pos, i, q);
            pos++;
        }

        // map y |-> i
        p = i & ~y;
        if (p) {
            update_permutation(perm, y, p);
            pos = gates.emplace(pos, y, p);
        }
        q = ~i & y;
        if (q) {
            update_permutation(perm, i, q);
            pos = gates.emplace(pos, i, q);
        }
    }
    return gates;
}

inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
    std::vector<Cbit> const& cbits, std::vector<uint32_t> const& perm)
{
    // GateList gates = unidirectional(perm);
    // GateList gates = bidirectional(perm);
    GateList gates = multidirectional(perm);
    for (auto [controls, targets] : gates) {
        std::vector<Qubit> cs;
        for (uint32_t c = 0; controls; controls >>= 1, ++c) {
            if (controls & 1) {
                cs.emplace_back(qubits.at(c));
            }
        }
        for (uint32_t t = 0; targets; targets >>= 1, ++t) {
            if ((targets & 1) == 0) {
                continue;
            }
            // FIXME: Quite hacky
            cs.push_back(qubits.at(t));
            circuit.apply_operator(Op::X(), {cs}, cbits);
            cs.pop_back();
        }
    }
}
}

void transform_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
    std::vector<Cbit> const& cbits, std::vector<uint32_t> const& perm)
{
    assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
    synthesize(circuit, qubits, cbits, perm);
}

Circuit transform_synth(std::vector<uint32_t> const& perm)
{
    assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
    Circuit circuit;
    uint32_t const num_qubits = __builtin_ctz(perm.size());
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    transform_synth(circuit, qubits, {}, perm);
    return circuit;
}

} // namespace tweedledum
