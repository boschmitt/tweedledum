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

// TODO: Find a better place to put this?!
// It's a bit annoying that mockturtle don't default flows

namespace tweedledum {

inline auto xag_simulate(mockturtle::xag_network xag)
{
    using namespace mockturtle;
    using TruthTable = kitty::dynamic_truth_table;
    using Simulator = default_simulator<TruthTable>;
    auto const results = simulate<TruthTable>(xag, Simulator(xag.num_pis()));
    return results;
}

} // namespace tweedledum
