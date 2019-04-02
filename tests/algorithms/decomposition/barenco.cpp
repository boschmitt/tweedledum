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
#include <tweedledum/networks/qubit.hpp>
#include <vector>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Decompose 3-controlled Toffoli gate", "[barenco][template]",
                           (gg_network, netlist), (mcmt_gate))
{
	TestType network;
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	network.add_gate(gate::mcx, std::vector<qubit_id>({0u, 1u, 2u}),
	                 std::vector<qubit_id>(1, 3u));
	auto snetwork = barenco_decomposition(network);
}
