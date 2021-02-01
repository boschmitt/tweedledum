/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/bidecomposition.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc2.hpp>
#include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/xag_optimization.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/properties/mccost.hpp>

// TODO: Find a better place to put this?!
// It's a bit annoying that mockturtle don't default flows

namespace mockturtle {
    template<class Ntk>
struct free_xor_cost
{
  uint32_t operator()( Ntk const& ntk, node<Ntk> const& n ) const
  {
    return ntk.is_xor( n ) ? 0 : 1;
  }
};
}

namespace tweedledum {

inline void xag_optimize(mockturtle::xag_network& xag)
{
    using namespace mockturtle;
    // Parameters: CUT rewriting using database
    future::xag_minmc_resynthesis resyn;
    cut_rewriting_params cut_rwrt_cfg;
    cut_rwrt_cfg.cut_enumeration_ps.cut_size = 5;

    // Parameters: Refactoring
    mockturtle::bidecomposition_resynthesis<xag_network> bi_resyn;
    refactoring_params r_params;
    r_params.allow_zero_gain = true;
    // r_params.use_dont_cares = tru;

    xag = xag_constant_fanin_optimization(xag);
    xag = cleanup_dangling(xag);
    xag = xag_dont_cares_optimization(xag);
    xag = cleanup_dangling(xag);

    refactoring(xag, bi_resyn, r_params, nullptr, free_xor_cost<xag_network>());
    // refactoring(xag, bi_resyn);
    xag = cleanup_dangling(xag);

    cut_rewriting_with_compatibility_graph(xag, resyn, cut_rwrt_cfg, nullptr, mc_cost<xag_network>());
    xag = cleanup_dangling(xag);

    xag = xag_constant_fanin_optimization(xag);
    xag = cleanup_dangling(xag);
    xag = xag_dont_cares_optimization(xag);
    xag = cleanup_dangling(xag);
}

} // namespace tweedledum
