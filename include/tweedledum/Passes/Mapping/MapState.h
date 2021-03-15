/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Wire.h"
#include "../../Target/Device.h"
#include "../Utility/shallow_duplicate.h"

#include <vector>

namespace tweedledum {

struct MapState {

    MapState(Circuit const& original, Device const& device)
    : device(device), original(original), mapped(shallow_duplicate(original))
    , v_to_phy(device.num_qubits(), Qubit::invalid())
    , phy_to_v(device.num_qubits(), Qubit::invalid())
    {
        for (uint32_t i = original.num_qubits(); i < device.num_qubits(); ++i) {
            mapped.create_qubit();
        }
    }

    void swap_qubits(Qubit const phy0, Qubit const phy1)
    {
        assert(device.are_connected(phy0, phy1));
        Qubit const v0 = phy_to_v.at(phy0);
        Qubit const v1 = phy_to_v.at(phy1);
        if (v0 != Qubit::invalid()) {
            v_to_phy.at(v0) = phy1;
        }
        if (v1 != Qubit::invalid()) {
            v_to_phy.at(v1) = phy0;
        }
        std::swap(phy_to_v.at(phy0), phy_to_v.at(phy1));
    }

    Device const& device;
    Circuit const& original;
    Circuit mapped;
    std::vector<Qubit> v_to_phy;
    std::vector<Qubit> phy_to_v;
};

} // namespace tweedledum
