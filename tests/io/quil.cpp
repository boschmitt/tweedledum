/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/gdg_network.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Read simple quil into a GG network", "[quil]")
{
	using namespace tweedledum;
	std::string input("RZ(-2.356194490192344) 1\n"
	                  "RX(pi/2) 1\n"
	                  "RZ(-2.3561944901923444) 2\n"
	                  "RX(pi/2) 2\n"
	                  "CZ 2 1\n"
	                  "RZ(0.07877076653175984) 1\n"
	                  "RZ(1.5150393470578472) 1\n");

	gg_network<mcst_gate> gg_net;
	read_quil_buffer(gg_net, input);
	CHECK(gg_net.size() == 11);
	CHECK(gg_net.num_qubits() == 2);
	CHECK(gg_net.num_gates() == 7);
}

TEST_CASE("Read simple quil into a GDG network", "[quil]")
{
	using namespace tweedledum;
	std::string input("RZ(-2.356194490192344) 1\n"
	                  "RX(pi/2) 1\n"
	                  "RZ(-2.3561944901923444) 2\n"
	                  "RX(pi/2) 2\n"
	                  "CZ 2 1\n"
	                  "RZ(0.07877076653175984) 1\n"
	                  "RZ(1.5150393470578472) 1\n");

	gdg_network<mcst_gate> gdg_net;
	read_quil_buffer(gdg_net, input);
	CHECK(gdg_net.size() == 11);
	CHECK(gdg_net.num_qubits() == 2);
	CHECK(gdg_net.num_gates() == 7);
}

TEST_CASE("Read simple quil into a netlist", "[quil]")
{
	using namespace tweedledum;
	std::string input("RZ(-2.356194490192344) 1\n"
	                  "RX(pi/2) 1\n"
	                  "RZ(-2.3561944901923444) 2\n"
	                  "RX(pi/2) 2\n"
	                  "CZ 2 1\n"
	                  "RZ(0.07877076653175984) 1\n"
	                  "RZ(1.5150393470578472) 1\n");

	netlist<mcst_gate> netlist;
	read_quil_buffer(netlist, input);
	CHECK(netlist.size() == 11);
	CHECK(netlist.num_qubits() == 2);
	CHECK(netlist.num_gates() == 7);
}

TEST_CASE("Write simple MCMT into quil", "[quil]")
{
	using namespace tweedledum;
	gg_network<mcmt_gate> gg_net;
	gg_net.add_qubit();
	gg_net.add_qubit();
	gg_net.add_qubit();
	auto controls = std::vector<uint32_t>({0u, 1u});
	auto target = std::vector<uint32_t>({2u});
	gg_net.add_gate(gate_kinds_t::mcx, controls, target);
	CHECK(gg_net.size() == 7);
	CHECK(gg_net.num_qubits() == 3);
	CHECK(gg_net.num_gates() == 1);

	std::ostringstream os;
	write_quil(gg_net, os);
	CHECK(os.str() == "CCNOT 0 1 2\n");
}
