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

    TruthTable(kitty::dynamic_truth_table const& tt, bool is_phase = false)
        : truth_table_(tt), is_phase_(is_phase)
    {}

    kitty::dynamic_truth_table const& truth_table() const
    {
        return truth_table_;
    }

    bool is_phase() const
    {
        return is_phase_;
    }

    uint32_t num_targets() const
    {
        if (is_phase_) {
            return truth_table_.num_vars();
        }
        return truth_table_.num_vars() + 1;
    }

    bool operator==(TruthTable const& other) const
    {
        return truth_table_ == other.truth_table_;
    }

private:
    kitty::dynamic_truth_table const truth_table_;
    bool const is_phase_;
};

} // namespace tweedledum
