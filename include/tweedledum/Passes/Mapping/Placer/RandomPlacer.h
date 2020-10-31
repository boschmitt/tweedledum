/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../../IR/Circuit.h"
#include "../../../IR/Wire.h"
#include "../../../Target/Device.h"
#include "../MapState.h"

#include <random>
#include <vector>

namespace tweedledum {

class RandomPlacer {
public:
    RandomPlacer(MapState& state) : state_(state)
    {}

    bool run(uint32_t seed = 17u)
    {
        // Initialize with the trivial placement
        for (uint32_t i = 0u; i < state_.mapped.num_qubits(); ++i) {
            state_.v_to_phy.at(i) = state_.mapped.wire_ref(i);
        }
        std::mt19937 rnd(seed);
        std::shuffle(state_.v_to_phy.begin(), state_.v_to_phy.end(), rnd);
        for (uint32_t i = 0u; i < state_.v_to_phy.size(); ++i) {
            state_.phy_to_v.at(state_.v_to_phy.at(i)) = state_.mapped.wire_ref(i);
        }
        return false;
    }

private:
    MapState& state_;
};

} // namespace tweedledum
