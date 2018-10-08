/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/mapping/relative_phase.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/write_qpic.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

TEST_CASE("Decompose 2-controlled Z gate", "[relative_phase]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> network;
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	network.add_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u}));
	auto snetwork = relative_phase_mapping(network);
	write_qpic(snetwork, "relative.qpic");
}
