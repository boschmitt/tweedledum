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
        // Initialize with the trivial placement
        std::vector<Qubit> phy_to_v(device_.num_qubits(), Qubit::invalid());
        for (uint32_t i = 0u; i < device_.num_qubits(); ++i) {
            phy_to_v.at(i) = Qubit(i);
        }
        std::mt19937 rnd(seed_);
        std::shuffle(phy_to_v.begin(), phy_to_v.end(), rnd);
        Placement placement(device_.num_qubits(), original_.num_qubits());
        placement.phy_to_v(phy_to_v);
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
