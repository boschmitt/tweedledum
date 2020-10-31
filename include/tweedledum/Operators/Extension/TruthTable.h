/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <kitty/kitty.hpp>
#include <string_view>

namespace tweedledum::Op {

class TruthTable {
public:
    static constexpr std::string_view kind()
    {
        return "ext.truth_table";
    }

    TruthTable(kitty::dynamic_truth_table const& tt)
        : truth_table_(tt)
    {}

    kitty::dynamic_truth_table const& truth_table() const
    {
        return truth_table_;
    }

private:
    kitty::dynamic_truth_table const truth_table_;
};

} // namespace tweedledum
