/* author: Fereshte */
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/qsp_tt.hpp>

using namespace tweedledum;

TEST_CASE("Prepare GHZ(3) state with qsp_tt method", "[qsp_tt]")
{
	tweedledum::netlist<tweedledum::mcst_gate> network;
        
	kitty::dynamic_truth_table tt(3);
	kitty::create_from_binary_string(tt, "10000001");

	qsp_tt_statistics stats;
	qsp_tt(network, tt, stats);

	CHECK(stats.total_cnots == 5u);
}
