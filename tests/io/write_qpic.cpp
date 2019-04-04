/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/write_qpic.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Write simple network into qpic", "[qpic][template]",
                           (gg_network, netlist), (mcmt_gate, mcst_gate))
{
	TestType network;
	auto q0 = network.add_qubit();
	network.add_cbit();
	auto q1 = network.add_qubit();
	auto q2 = network.add_qubit();
	std::vector<io_id> controls = {q0, q1};
	std::vector<io_id> target = {q2};
	network.add_gate(gate::mcx, controls, target);

	std::ostringstream os;
	write_qpic(network, os);
	CHECK(os.str() == "id0 W q0 q0\nid1 W c0 c0 cwire\nid2 W q1 q1\nid3 W q2 q2\n+id3  id0 id2\n");
}