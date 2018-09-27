/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/dotqc.hpp>
#include <tweedledum/networks/gdg_network.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Read simple dotqc into a GG network", "[dotqc]")
{
	using namespace tweedledum;
	std::istringstream input;
	input.str(".v q0 q1\n"
	          ".i q0 q1\n"
	          "BEGIN\n"
	          "H q0\n"
	          "tof q0 q1\n"
	          "T q0\n"
	          "tof q0 q1\n"
	          "END\n");

	gg_network<mcst_gate> gg_net;
	dotqc_reader reader(gg_net);
	dotqc_read(input, reader, identify_gate_kind());
	CHECK(gg_net.size() == 8);
	CHECK(gg_net.num_qubits() == 2);
	CHECK(gg_net.num_gates() == 4);
}

TEST_CASE("Read simple dotqc into a GDG network", "[dotqc]")
{
	using namespace tweedledum;
	std::istringstream input;
	input.str(".v q0 q1\n"
	          ".i q0 q1\n"
	          "BEGIN\n"
	          "H q0\n"
	          "tof q0 q1\n"
	          "T q0\n"
	          "tof q0 q1\n"
	          "END\n");

	gdg_network<mcst_gate> gdg_net;
	dotqc_reader reader(gdg_net);
	dotqc_read(input, reader, identify_gate_kind());
	CHECK(gdg_net.size() == 8);
	CHECK(gdg_net.num_qubits() == 2);
	CHECK(gdg_net.num_gates() == 4);
}

TEST_CASE("Read simple dotqc into a netlist", "[dotqc]")
{
	using namespace tweedledum;
	std::istringstream input;
	input.str(".v q0 q1\n"
	          ".i q0 q1\n"
	          "BEGIN\n"
	          "H q0\n"
	          "tof q0 q1\n"
	          "T q0\n"
	          "tof q0 q1\n"
	          "END\n");

	netlist<mcst_gate> netlist;
	dotqc_reader reader(netlist);
	dotqc_read(input, reader, identify_gate_kind());
	CHECK(netlist.size() == 8);
	CHECK(netlist.num_qubits() == 2);
	CHECK(netlist.num_gates() == 4);
}
