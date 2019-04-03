/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/gate_base.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Common functionality for all networks", "[networks][template]",
                           (gg_network, netlist), (mcst_gate, mcmt_gate))
{
	SECTION("No gates")
	{
		TestType network;
		CHECK(network.size() == 0);
		network.add_qubit("q0");
		network.add_qubit();
		CHECK(network.size() == 4);
		CHECK(network.num_qubits() == 2);
	}
	SECTION("With gates")
	{
		TestType network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::cx, q0, q1);
		CHECK(network.size() == 6);
		CHECK(network.num_qubits() == 2);
	}
}