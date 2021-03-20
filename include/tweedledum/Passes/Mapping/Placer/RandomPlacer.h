/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Wire.h"
#include "../../../Target/Device.h"
#include "../../../Target/Placement.h"

#include <random>
#include <vector>

namespace tweedledum {

class RandomPlacer {
public:
    RandomPlacer(Device const& device, Circuit const& original)
        : device_(device)
        , original_(original)
        , seed_(17u)
    {}

    std::optional<Placement> run()
    {
        std::vector<Qubit> phys;
        for (uint32_t i = 0u; i < device_.num_qubits(); ++i) {
            phys.emplace_back(i);
        }
        std::mt19937 rnd(seed_);
        std::shuffle(phys.begin(), phys.end(), rnd);

        Placement placement(device_.num_qubits(), original_.num_qubits());
        for (uint32_t i = 0u; i < original_.num_qubits(); ++i) {
            placement.map_v_phy(Qubit(i), phys.at(i));
        }
        return placement;
    }

private:
    Device const& device_;
    Circuit const& original_;
    uint32_t seed_;
};

/*! \brief Yet to be written.
 */
std::optional<Placement> random_place(Device const& device,
    Circuit const& original);

} // namespace tweedledum
