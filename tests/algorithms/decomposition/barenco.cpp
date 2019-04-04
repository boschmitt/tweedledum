/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/decomposition/barenco.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <vector>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Decompose 3-controlled Toffoli gate", "[barenco][template]",
                           (gg_network, netlist), (mcmt_gate))
{
	TestType network;
	auto q0 = network.add_qubit();
	auto q1 = network.add_qubit();
	auto q2 = network.add_qubit();
	auto q3 = network.add_qubit();
	network.add_gate(gate::mcx, std::vector<io_id>({q0, q1, q2}),
	                 std::vector<io_id>(q1, q3));
	auto snetwork = barenco_decomposition(network);
}
