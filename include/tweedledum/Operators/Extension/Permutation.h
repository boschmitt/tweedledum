/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cmath>
#include <string_view>
#include <vector>

namespace tweedledum::Op {

class Permutation {
public:
    static constexpr std::string_view kind()
    {
        return "ext.permutation";
    }

    Permutation(std::vector<uint32_t> const& perm)
        : permutation_(perm)
    {}

    std::vector<uint32_t> const& permutation() const
    {
        return permutation_;
    }

    uint32_t num_targets() const
    {
        return std::log2(permutation_.size());
    }

    bool operator==(Permutation const& other) const
    {
        return permutation_ == other.permutation_;
    }

private:
    std::vector<uint32_t> const permutation_;
};

} // namespace tweedledum::Op
