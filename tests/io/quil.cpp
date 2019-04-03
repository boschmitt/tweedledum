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
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Write simple network into quil", "[quil][template]",
                           (gg_network, netlist), (mcmt_gate, mcst_gate))
{
	TestType network;
	network.add_qubit();
	network.add_qubit();
	network.add_qubit();
	std::vector<io_id> controls = {io_id(0), io_id(1)};
	std::vector<io_id> target = {io_id(2)};
	network.add_gate(gate::mcx, controls, target);
	CHECK(network.size() == 7);
	CHECK(network.num_qubits() == 3);
	CHECK(network.num_gates() == 1);

	std::ostringstream os;
	write_quil(network, os);
	CHECK(os.str() == "CCNOT 0 1 2\n");
}