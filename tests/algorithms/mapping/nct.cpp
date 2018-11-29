/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/mapping/nct.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

TEST_CASE("Decompose 3-controlled Toffoli gate", "[nct]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> network;
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	network.add_gate(gate_set::mcx, std::vector({0u, 1u, 2u}), std::vector({3u}));
	auto snetwork = nct_mapping(network);
}
