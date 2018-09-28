/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <sstream>
#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gdg_network.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pprm", "[syn_stg]")
{
	using namespace tweedledum;
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	// Looks bad :(
	gg_network<mcmt_gate> gg_net = stg_from_pprm2<gg_network<mcmt_gate>>(tt);
}

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pkrm", "[syn_stg]")
{
	using namespace tweedledum;
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	gg_network<mcmt_gate> gg_net = stg_from_pkrm2<gg_network<mcmt_gate>>(tt);
}

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_spectrum", "[syn_stg]")
{
	using namespace tweedledum;
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	gg_network<mcmt_gate> gg_net = stg_from_spectrum2<gg_network<mcmt_gate>>(tt);
}