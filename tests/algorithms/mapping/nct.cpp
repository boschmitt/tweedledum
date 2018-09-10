/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/mapping/nct.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/networks/gates/mct_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Decompose 3-controlled Toffoli gate", "nct")
{
	using namespace tweedledum;
	netlist<mct_gate> network, snetwork;
	network.allocate_qubit();
	network.allocate_qubit();
	network.allocate_qubit();
	network.allocate_qubit();
	network.add_multiple_controlled_gate(gate_kinds_t::mcx, {3, 0, 1, 2});
	nct_mapping(snetwork, network);
}
