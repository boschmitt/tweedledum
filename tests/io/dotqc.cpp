/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/dotqc.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Read simple dotqc into a network", "[dotqc][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	std::istringstream input;
	input.str(".v q0 q1\n"
	          ".i q0 q1\n"
	          "BEGIN\n"
	          "H q0\n"
	          "tof q0 q1\n"
	          "T q0\n"
	          "tof q0 q1\n"
	          "END\n");

	TestType network;
	dotqc_reader reader(network);
	dotqc_read(input, reader, identify_gate());
	CHECK(network.size() == 8);
	CHECK(network.num_qubits() == 2);
	CHECK(network.num_gates() == 4);
}
