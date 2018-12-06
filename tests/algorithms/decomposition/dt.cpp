/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/decomposition/dt.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

TEST_CASE("Decompose 2-controlled Z gate", "[dt_decomposition]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> network;
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	network.add_gate(gate::mcz, std::vector({0u, 1u}), std::vector(1, 2u));
	auto snetwork = dt_decomposition(network);
}