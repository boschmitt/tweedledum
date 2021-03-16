/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Qubit.h"

#include <vector>

namespace tweedledum {

class Placement {
public:
    Placement(uint32_t num_phy_qubits, uint32_t num_v_qubits)
        : v_to_phy_(num_v_qubits, Qubit::invalid())
        , phy_to_v_(num_phy_qubits, Qubit::invalid())
    {} 

    void reset()
    {
        std::fill(v_to_phy_.begin(), v_to_phy_.end(), Qubit::invalid());
        std::fill(phy_to_v_.begin(), phy_to_v_.end(), Qubit::invalid());
    }

    Qubit v_to_phy(Qubit const v) const
    {
        return v_to_phy_.at(v);
    }

    Qubit phy_to_v(Qubit const phy) const
    {
        return phy_to_v_.at(phy);
    }

    // This function only for for mapping v <-> phy when they have not already
    // been mapped before! TODO: protect it with an assertion
    void map_v_phy(Qubit const v, Qubit const phy)
    {
        assert(v != Qubit::invalid() || phy != Qubit::invalid());
        if (v != Qubit::invalid()) {
            v_to_phy_.at(v) = phy;
        }
        if (phy != Qubit::invalid()) {
            phy_to_v_.at(phy) = v;
        }
    }

    std::vector<Qubit> const& v_to_phy() const
    {
        return v_to_phy_;
    }

    std::vector<Qubit> const& phy_to_v() const
    {
        return phy_to_v_;
    }

    void swap_qubits(Qubit const phy0, Qubit const phy1)
    {
        Qubit const v0 = phy_to_v_.at(phy0);
        Qubit const v1 = phy_to_v_.at(phy1);
        if (v0 != Qubit::invalid()) {
            v_to_phy_.at(v0) = phy1;
        }
        if (v1 != Qubit::invalid()) {
            v_to_phy_.at(v1) = phy0;
        }
        std::swap(phy_to_v_.at(phy0), phy_to_v_.at(phy1));
    }

    bool operator==(Placement const& other) const
    {
        return v_to_phy_ == other.v_to_phy_ && phy_to_v_ == other.phy_to_v_;
    }

private:
    std::vector<Qubit> v_to_phy_;
    std::vector<Qubit> phy_to_v_;
};

} // namespace tweedledum
