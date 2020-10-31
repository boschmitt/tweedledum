/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Wire.h"
#include "../../Target/Device.h"

#include <vector>

namespace tweedledum {

struct MapState {

    MapState(Circuit const& original, Device const& device)
    : device(device), original(original)
    , wire_to_v(original.num_wires(), WireRef::invalid())
    , v_to_phy(device.num_qubits(), WireRef::invalid())
    , phy_to_v(device.num_qubits(), WireRef::invalid())
    {
        original.foreach_wire([&](Wire const& wire) {
            if (wire.kind == Wire::Kind::quantum) {
                WireRef const new_wire = mapped.create_qubit(wire.name);
                wire_to_v.at(wire) = new_wire;
            }
        });
        for (uint32_t i = original.num_qubits(); i < device.num_qubits(); ++i) {
            mapped.create_qubit();
        }
        original.foreach_wire([&](Wire const& wire) {
            if (wire.kind == Wire::Kind::classical) {
                wire_to_v.at(wire) = mapped.create_cbit(wire.name);
            }
        });
    }

    WireRef wire_to_wire(WireRef const ref) const
    {
        if (ref.kind() == Wire::Kind::quantum) {
            return v_to_phy.at(wire_to_v.at(ref));
        }
        return wire_to_v.at(ref);
    }

    void swap_qubits(WireRef const phy0, WireRef const phy1)
    {
        assert(device.are_connected(phy0, phy1));
        WireRef const v0 = phy_to_v.at(phy0);
        WireRef const v1 = phy_to_v.at(phy1);
        if (v0 != WireRef::invalid()) {
            v_to_phy.at(v0) = phy1;
        }
        if (v1 != WireRef::invalid()) {
            v_to_phy.at(v1) = phy0;
        }
        std::swap(phy_to_v.at(phy0), phy_to_v.at(phy1));
    }

    Device const& device;
    Circuit const& original;
    Circuit mapped;
    std::vector<WireRef> wire_to_v;
    std::vector<WireRef> v_to_phy;
    std::vector<WireRef> phy_to_v;
};

} // namespace tweedledum
