/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/views/layers_view.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Layers view", "[layers_view][template]",
                           (gg_network), (mcmt_gate, io3_gate))
{
	TestType network;
	SECTION("Empty network")
	{
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 0u);
		CHECK(layered_ntk.num_layers() == 0u);
	}
	const io_id q0 = network.add_qubit();
	const io_id q1 = network.add_qubit();
	const io_id q2 = network.add_qubit();
	SECTION("With qubits, but no gates")
	{
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 0u);
		CHECK(layered_ntk.num_layers() == 2u);
		CHECK(layered_ntk.layer(0).size() == 3u);
		CHECK(layered_ntk.layer(1).size() == 3u);
	}
	SECTION("One layer of gates")
	{
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::cx, q1, q2);
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 1u);
		CHECK(layered_ntk.num_layers() == 3u);
		CHECK(layered_ntk.layer(0).size() == 3u);
		CHECK(layered_ntk.layer(1).size() == 2u);
		CHECK(layered_ntk.layer(2).size() == 3u);
	}
	SECTION("Two layer of gates")
	{
		network.add_gate(gate::hadamard, q2);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q2, q1);
		network.add_gate(gate::hadamard, q0);
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 2u);
		CHECK(layered_ntk.num_layers() == 4u);
		CHECK(layered_ntk.layer(0).size() == 3u);
		CHECK(layered_ntk.layer(1).size() == 2u);
		CHECK(layered_ntk.layer(2).size() == 2u);
		CHECK(layered_ntk.layer(3).size() == 3u);
	}
	SECTION("All outputs are in the last layer")
	{
		const io_id q3 = network.add_qubit();
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q0, q3);
		network.add_gate(gate::hadamard, q3);
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 4u);
		CHECK(layered_ntk.num_layers() == 6u);
		CHECK(layered_ntk.layer(0).size() == 4u);
		CHECK(layered_ntk.layer(1).size() == 1u);
		CHECK(layered_ntk.layer(2).size() == 1u);
		CHECK(layered_ntk.layer(3).size() == 1u);
		CHECK(layered_ntk.layer(4).size() == 1u);
		CHECK(layered_ntk.layer(5).size() == 4u);
	}
}