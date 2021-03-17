/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Optimization/linear_resynth.h"

#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Extension/Parity.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"
#include "tweedledum/Synthesis/linear_synth.h"

namespace tweedledum {
namespace {

struct Slice {
    std::vector<InstRef> linear_gates;
    std::vector<InstRef> non_linear_gates;
};

inline std::vector<Slice> partition_into_silces(Circuit const& original)
{
    std::vector<uint32_t> to_slice(original.size(), 0);
    std::vector<Slice> slices;
    original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        uint32_t max = 0;
        inst.foreach_qubit([&](InstRef child) {
            max = std::max(max, to_slice[child]);
        });
        to_slice[ref] = max;
        if (max == slices.size()) {
            slices.emplace_back();
        }
        if (!inst.is_one<Op::X, Op::Parity>()) { 
            slices.at(max).non_linear_gates.emplace_back(ref);
            to_slice[ref] += 1;
        } else if (inst.is_a<Op::X>() && inst.num_wires() > 2u) {
            slices.at(max).non_linear_gates.emplace_back(ref);
            to_slice[ref] += 1;
        } else {
            slices.at(max).linear_gates.emplace_back(ref);
        }
    });
    return slices;
}

inline void resynth_slice(Circuit const& original, Slice const& slice,
    Circuit& result, nlohmann::json const& config)
{
    if (!slice.linear_gates.empty()) {
        // Get the qubits
        std::vector<int32_t> to_id(original.num_wires(), -1);
        std::vector<Qubit> qubits;
        for (InstRef index : slice.linear_gates) {
            Instruction const& inst = original.instruction(index);
            inst.foreach_qubit([&](Qubit wref) {
                if (to_id[wref] != -1) {
                    return;
                }
                to_id[wref] = qubits.size();
                qubits.push_back(wref);
            });
        }
        // Create matrix
        BMatrix matrix = BMatrix::Identity(qubits.size(), qubits.size());
        uint32_t num_cnot = 0u;
        for (InstRef index : slice.linear_gates) {
            Instruction const& inst = original.instruction(index);
            int32_t const target = to_id.at(inst.target());
            inst.foreach_control([&](Qubit wref) {
                int32_t const control = to_id.at(wref);
                matrix.row(target) += matrix.row(control);
                num_cnot += 1u;
            });
        }
        // Synthesize matrix
        // Circuit linear = linear_synth(result, qubits, matrix, config);
        Circuit subcircuit = linear_synth(matrix, config);
        if (subcircuit.size() < num_cnot) {
            result.append(subcircuit, qubits, {});
        } else {
            for (InstRef index : slice.linear_gates) {
                Instruction const& inst = original.instruction(index);
                result.apply_operator(inst);
            }
        }
    }
    // Add Toffoli gates
    for (InstRef index : slice.non_linear_gates) {
        Instruction const& inst = original.instruction(index);
        result.apply_operator(inst);
    }
}

} // namespace linear_resynth_detail
#pragma endregion

Circuit linear_resynth(Circuit const& original, nlohmann::json const& config)
{
    Circuit result = shallow_duplicate(original);
    auto slices = partition_into_silces(original);
    for (auto const& slice : slices) {
        resynth_slice(original, slice, result, config);
    }
    return result;
}

} // namespace tweedledum
