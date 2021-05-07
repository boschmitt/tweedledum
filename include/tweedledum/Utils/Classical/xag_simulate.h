/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <fmt/format.h>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc2.hpp>
#include <mockturtle/algorithms/xag_optimization.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/properties/mccost.hpp>
#include <vector>

namespace tweedledum {

inline std::vector<kitty::dynamic_truth_table> xag_simulate(
  mockturtle::xag_network const& xag)
{
    using namespace mockturtle;
    using TruthTable = kitty::dynamic_truth_table;
    using Simulator = default_simulator<TruthTable>;
    auto const results = simulate<TruthTable>(xag, Simulator(xag.num_pis()));
    return results;
}

inline std::vector<bool> xag_simulate(
  mockturtle::xag_network const& xag, std::vector<bool> const& pattern)
{
    using namespace mockturtle;
    using Simulator = default_simulator<bool>;
    auto const results = simulate<bool>(xag, Simulator(pattern));
    return results;
}

} // namespace tweedledum
