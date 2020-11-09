/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Optimization/phase_folding.h"
#include "tweedledum/Operators/All.h"
#include "tweedledum/Operators/Utils.h"
#include "tweedledum/Utils/LinearPP.h"

namespace tweedledum {


Circuit phase_folding(Circuit const& original)
{
    using ESOP = std::vector<uint32_t>;
    constexpr uint32_t qid_max = std::numeric_limits<uint32_t>::max();

    Circuit optimized;
    uint32_t num_path_vars = 1u;
    std::vector<uint32_t> wire_to_qid(original.num_wires(), qid_max);
    std::vector<ESOP> qubit_pathsum;
    std::vector<uint8_t> skipped(original.size(), 0);

    original.foreach_wire([&](WireRef ref, Wire const& wire) {
        if (ref.kind() == Wire::Kind::classical) {
            optimized.create_cbit(wire.name);
            return;
        };
        optimized.create_qubit(wire.name);
        wire_to_qid.at(ref) = qubit_pathsum.size();
        qubit_pathsum.emplace_back(1u, (num_path_vars++ << 1));
    });

    LinearPP parities;
    original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        uint32_t const t = wire_to_qid.at(inst.target(0u));
        Angle const angle = rotation_angle(inst);
        if (inst.num_cbits() || (inst.num_qubits() > 2)) {
            goto new_vars_end;
        }
        if (inst.num_targets() == 2) {
            if (inst.is_one<Op::Swap>()) {
                uint32_t const t1 = wire_to_qid.at(inst.target(1u));
                std::swap(qubit_pathsum.at(t), qubit_pathsum.at(t1));
                return;
            }
            goto new_vars_end;
        }
        if (inst.is_one<Op::X>()) {
            if (inst.num_controls() == 0) {
                if (qubit_pathsum.at(t).at(0) == 1u) {
                    qubit_pathsum.at(t).erase(qubit_pathsum.at(t).begin());
                    return;
                }
                qubit_pathsum.at(t).insert(qubit_pathsum.at(t).begin(), 1u);
                return;
            }
            uint32_t c = wire_to_qid.at(inst.control());
            std::vector<uint32_t> esop;
            std::set_symmetric_difference(qubit_pathsum.at(c).begin(),
                qubit_pathsum.at(c).end(), qubit_pathsum.at(t).begin(),
                qubit_pathsum.at(t).end(), std::back_inserter(esop));
            qubit_pathsum.at(t) = esop;
            return;
        }
        if (angle != sym_angle::zero) {
            parities.add_term(qubit_pathsum.at(t), angle);
        }
        return;

    new_vars_end:
        skipped.at(ref) = 1u;
        inst.foreach_target([&](WireRef wref) {
            uint32_t const qubit = wire_to_qid.at(wref); 
            qubit_pathsum.at(qubit).clear();
            qubit_pathsum.at(qubit).emplace_back((num_path_vars++ << 1));
        });
    });

    num_path_vars = 1u;
    for (uint32_t i = 0; i < original.num_qubits(); ++i) {
        qubit_pathsum.at(i) = {(num_path_vars++ << 1)};
    }
    
    original.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        if (skipped.at(ref)) {
            inst.foreach_target([&](WireRef wref) {
                uint32_t const qubit = wire_to_qid.at(wref); 
                qubit_pathsum.at(qubit).clear();
                qubit_pathsum.at(qubit).emplace_back((num_path_vars++ << 1));
            });
            optimized.apply_operator(inst);
        }
        // Rotations
        if (inst.is_one<Op::P, Op::S, Op::Sdg, Op::T, Op::Tdg, Op::Z>()) {
            return;
        }

        uint32_t const t = wire_to_qid.at(inst.target());
        if (inst.is_one<Op::Swap>()) {
            uint32_t const t1 = wire_to_qid.at(inst.target(1u));
            std::swap(qubit_pathsum.at(t), qubit_pathsum.at(t1));
        }
        if (inst.is_one<Op::X>()) {
            if (inst.num_controls() == 0) {
                if (qubit_pathsum.at(t).at(0) == 1u) {
                    qubit_pathsum.at(t).erase(qubit_pathsum.at(t).begin());
                }
                qubit_pathsum.at(t).insert(qubit_pathsum.at(t).begin(), 1u);
            } else {
                uint32_t c = wire_to_qid.at(inst.control());
                std::vector<uint32_t> esop;
                std::set_symmetric_difference(qubit_pathsum.at(c).begin(),
                    qubit_pathsum.at(c).end(), qubit_pathsum.at(t).begin(),
                    qubit_pathsum.at(t).end(), std::back_inserter(esop));
                qubit_pathsum.at(t) = esop;
            }
        }
        optimized.apply_operator(inst);
        Angle const angle = parities.extract_term(qubit_pathsum.at(t));
        if (angle == sym_angle::zero) {
            return;
        }
        apply_identified_phase(optimized, angle, inst.target());
    });
    return optimized;
}

} // namespace tweedledum
