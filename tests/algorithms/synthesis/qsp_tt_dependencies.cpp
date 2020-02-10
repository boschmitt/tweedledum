/* author: Fereshte */
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/qsp_tt_dependencies.hpp>

using namespace tweedledum;

TEST_CASE("Prepare GHZ(3) state with qsp_tt_dependencies method", "[qsp_tt_dependencies]")
{
	tweedledum::netlist<tweedledum::mcst_gate> network;
        
	kitty::dynamic_truth_table tt(3);
	kitty::create_from_binary_string(tt, "10000001");

	std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> deps;
	deps[0].emplace_back(std::make_pair("eq", std::vector<uint32_t>{4}));
	deps[1].emplace_back(std::make_pair("eq", std::vector<uint32_t>{4}));

	qsp_tt_deps_statistics stats;
	qsp_tt_dependencies(network, tt, deps, stats);

	CHECK(stats.total_cnots == 2u);
}

TEST_CASE("Prepare w(3) state with qsp_tt_dependencies method", "[qsp_tt_dependencies]")
{
	tweedledum::netlist<tweedledum::mcst_gate> network;
        
	kitty::dynamic_truth_table tt(3);
	kitty::create_from_binary_string(tt, "01101000");

	std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>> deps;
	deps[0].emplace_back(std::make_pair("xnor", std::vector<uint32_t>{4,2}));

	qsp_tt_deps_statistics stats;
	qsp_tt_dependencies(network, tt, deps, stats);

	CHECK(stats.total_cnots == 4u);
}