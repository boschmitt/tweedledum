/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../Operators/Standard/Measure.h"
#include "../../Utils/Cut.h"

#include <algorithm>
#include <limits>
#include <vector>

namespace tweedledum {

inline std::vector<Cut> compute_cuts(
  Circuit const& circuit, uint32_t const cut_width = 2u)
{
    constexpr int32_t invalid_cut = std::numeric_limits<int32_t>::min();
    std::vector<int32_t> inst_cut(circuit.num_instructions(), invalid_cut);
    std::vector<Cut> cuts;
    circuit.foreach_instruction([&](InstRef ref) {
        Instruction const& inst = circuit.instruction(ref);
        if (inst.num_qubits() > cut_width || inst.is_a<Op::Measure>()) {
            inst_cut.at(ref) = -(cuts.size());
            cuts.emplace_back(inst.qubits(), inst.cbits(), ref);
            return;
        }
        int32_t cut = invalid_cut;
        uint32_t same_cut = 0;
        circuit.foreach_child(ref, [&](InstRef child) {
            if (cut == invalid_cut) {
                cut = inst_cut.at(child);
                return;
            }
            same_cut +=
              (cut == inst_cut.at(child) || cut < 0 || inst_cut.at(child) < 0);
            cut = std::max(cut, inst_cut.at(child));
        });
        if (++same_cut == inst.num_wires() && cut >= 0) {
            inst_cut.at(ref) = cut;
            cuts.at(cut).add_intruction(ref, inst);
            return;
        } else if (cut >= 0 && cuts.at(cut).num_qubits() < inst.num_qubits()) {
            inst_cut.at(ref) = cut;
            cuts.at(cut).add_intruction(ref, inst);
            return;
        }
        inst_cut.at(ref) = cuts.size();
        cuts.emplace_back(inst.qubits(), inst.cbits(), ref);
    });
    // Try to merge some cuts
    for (uint32_t i = 0u; i < cuts.size(); ++i) {
        if (cuts.at(i).num_qubits() >= cut_width) {
            continue;
        }
        for (uint32_t j = i + 1; j < cuts.size(); ++j) {
            int32_t did_merge =
              try_merge_cuts(cuts.at(i), cuts.at(j), cut_width);
            if (did_merge) {
                break;
            }
        }
    }
    // After merging some cuts, remove the empty ones
    auto end = std::remove_if(
      cuts.begin(), cuts.end(), [](Cut const& cut) { return cut.empty(); });
    cuts.erase(end, cuts.end());
    return cuts;
}

} // namespace tweedledum
