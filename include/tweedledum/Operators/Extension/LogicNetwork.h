/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <mockturtle/networks/xag.hpp>
#include <string_view>

namespace tweedledum::Op {

class LogicNetwork {
public:
    static constexpr std::string_view kind()
    {
        return "ext.logic_network";
    }

    LogicNetwork(mockturtle::xag_network const& xag)
        : logic_network_(xag)
    {}

    mockturtle::xag_network const& logic_network() const
    {
        return logic_network_;
    }

    uint32_t num_targets() const
    {
        return logic_network_.num_pos() + logic_network_.num_pis();
    }

    // FIXME: This is complicated.
    // Should this be:
    //   *) equivalence checking ? (pretty expensive)
    //   *) check if pointing to the same thing
    bool operator==([[maybe_unused]] LogicNetwork const& other) const
    {
        return false;
    }

private:
    mockturtle::xag_network const logic_network_;
};

} // namespace tweedledum::Op
