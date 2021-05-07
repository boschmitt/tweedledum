/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <fmt/format.h>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/properties/mccost.hpp>

namespace tweedledum {

inline void xag_stats(mockturtle::xag_network const& xag)
{
    using namespace mockturtle;
    auto mc_size = multiplicative_complexity(xag);
    auto mc_depth = multiplicative_complexity_depth(xag);
    fmt::print("[i] + [i/o] = {}/{}, nodes = {}\n", xag.num_pis(),
      xag.num_pos(), xag.size());
    fmt::print("[i] + mult. compl. size: {}\n",
      mc_size ? std::to_string(*mc_size) : "N/A");
    fmt::print("[i] + mult. compl. depth: {}\n\n",
      mc_depth ? std::to_string(*mc_depth) : "N/A");
}

} // namespace tweedledum
