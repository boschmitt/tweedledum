/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Optimization/steiner_resynth.h"

#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Extension/Bridge.h"
#include "tweedledum/Operators/Extension/Parity.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"
#include "tweedledum/Synthesis/steiner_gauss_synth.h"

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
        inst.foreach_qubit(
          [&](InstRef child) { max = std::max(max, to_slice[child]); });
        to_slice[ref] = max;
        if (max == slices.size()) {
            slices.emplace_back();
        }
        if (!inst.is_one<Op::Bridge, Op::X, Op::Swap, Op::Parity>()) {
            slices.at(max).non_linear_gates.emplace_back(ref);
            to_slice[ref] += 1;
        } else if (inst.is_a<Op::X>() && inst.num_wires() != 2u) {
            slices.at(max).non_linear_gates.emplace_back(ref);
            to_slice[ref] += 1;
        } else {
            slices.at(max).linear_gates.emplace_back(ref);
        }
    });
    return slices;
}

inline void resynth_slice(Circuit const& original, Device const& device,
  Slice const& slice, Circuit& result, nlohmann::json const& config)
{
    if (!slice.linear_gates.empty()) {
        // Create matrix
        BMatrix matrix =
          BMatrix::Identity(original.num_qubits(), original.num_qubits());
        uint32_t num_cnot = 0u;
        for (InstRef index : slice.linear_gates) {
            Instruction const& inst = original.instruction(index);
            int32_t const target = inst.target();
            if (inst.is_a<Op::Swap>()) {
                matrix.row(target).swap(matrix.row(inst.target(1u)));
                num_cnot += 3u;
                continue;
            }
            inst.foreach_control([&](Qubit wref) {
                int32_t const control = wref;
                matrix.row(target) += matrix.row(control);
                num_cnot += 1u;
            });
        }
        // Synthesize matrix
        Circuit subcircuit = steiner_gauss_synth(device, matrix, config);
        if (subcircuit.size() < num_cnot) {
            result.append(subcircuit, result.qubits(), {});
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

} // namespace

Circuit steiner_resynth(
  Circuit const& original, Device const& device, nlohmann::json const& config)
{
    Circuit result = shallow_duplicate(original);
    auto slices = partition_into_silces(original);
    for (auto const& slice : slices) {
        resynth_slice(original, device, slice, result, config);
    }
    return result;
}

} // namespace tweedledum
