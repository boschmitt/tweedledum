/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Passes/Mapping/Placer/TrivialPlacer.h"
#include "tweedledum/Target/Mapping.h"

using namespace tweedledum;

namespace {
using Sum = std::vector<uint32_t>;

// Fake pathsums can be employed to verify mappings.  Assuming that the mapping
// do not screw-up adding single qubit gates, we can safely ignore them and
// verify if the set of output path sums of the original circuit mataches the
// set of output path sums of the mapped circuit.
//
std::vector<Sum> fake_pathsums(
  Circuit const& circuit, Placement const& placement)
{
    std::vector<Sum> fake_pathsum;
    std::vector<Qubit> const& phy_to_v = placement.phy_to_v();
    for (uint32_t i = 0; i < phy_to_v.size(); ++i) {
        fake_pathsum.emplace_back(1u, phy_to_v.at(i));
    }
    circuit.foreach_instruction([&](Instruction const& inst) {
        if (inst.num_qubits() != 2u) {
            return;
        }
        Qubit const t = inst.target();
        if (inst.num_targets() == 2u && inst.is_one<Op::Swap>()) {
            Qubit const t1 = inst.target(1u);
            std::swap(fake_pathsum.at(t), fake_pathsum.at(t1));
            return;
        }
        Qubit const c = inst.control();
        std::vector<uint32_t> result;
        std::set_symmetric_difference(fake_pathsum.at(c).begin(),
          fake_pathsum.at(c).end(), fake_pathsum.at(t).begin(),
          fake_pathsum.at(t).end(), std::back_inserter(result));
        fake_pathsum.at(t) = result;
    });
    return fake_pathsum;
}
} // namespace

/*! \brief Verify if a circuit was correctly mapped (under assumptions, see
 * details).
 *
 * This method uses a trick to verify if a circuit has been correctly mapped.
 * It will consider all two-qubit gates that are not a SWAP to be CX and ignore
 * one-qubit gates, meaning that the circuits will be treated as reversible
 * circuits composed of CX and SWAP gates.  The algorithm basically checks if
 * the outputs are equal up to a permutation.
 *
 * NOTE: as it ignores one-qubit gates, this verification assumes that those
 *       gates were correctly mapped!
 *
 * \tparam Circuit the circuit type.
 * \param[in] original the unmapped circuit.
 * \param[in] mapped the mapped version of the orignal circuit.
 * \returns true if the circuit is correctly mapped under the assumptions.
 */
inline bool check_mapping(Device const& device, Circuit const& original,
  Circuit const& mapped, Mapping const& mapping)
{
    auto trivial_placement = trivial_place(device, original);
    auto original_pathsums = fake_pathsums(original, *trivial_placement);
    auto mapped_pathsums = fake_pathsums(mapped, mapping.init_placement);
    return std::is_permutation(original_pathsums.begin(),
      original_pathsums.end(), mapped_pathsums.begin());
}
