/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/decomposition/dt.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <vector>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Database decompostion", "[dt_decomposition][template]",
                           (gg_network, netlist), (mcmt_gate))
{
	TestType network;
	auto q0 = network.add_qubit();
	auto q1 = network.add_qubit();
	auto q2 = network.add_qubit();
	SECTION("Decompose 2-controlled Z gate")
	{
		network.add_gate(gate::mcx, std::vector<io_id>({q0, q1}),
		                 std::vector<io_id>(q1, q2));
		network.add_gate(gate::mcz, std::vector<io_id>({q0, q1}),
		                 std::vector<io_id>(q1, q2));
		auto snetwork = dt_decomposition(network);
	}
	SECTION("Decompose toffoli gate with one negative control")
	{
		network.add_gate(gate::mcx, std::vector({!q0, q1}), std::vector({q2}));
		auto snetwork = dt_decomposition(network);
	}
}
