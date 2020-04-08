/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/views/layers_view.hpp"

#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire.hpp"
#include "tweedledum/operations/w2_op.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Layers view", "[layers_view][views]", (op_dag), (w2_op, w3_op, wn32_op))
{
	TestType network;
	SECTION("Empty network") {
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 0u);
		CHECK(layered_ntk.num_layers() == 0u);
	}

	wire::id const q0 = network.create_qubit();
	wire::id const q1 = network.create_qubit();
	wire::id const q2 = network.create_qubit();
	SECTION("With qubits, but no gates") {
		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 0u);
		CHECK(layered_ntk.num_layers() == 1u);
		CHECK(layered_ntk.layer(0).size() == 3u);
	}
	SECTION("One layer of gates") {
		network.create_op(gate_lib::h, q0);
		network.create_op(gate_lib::cx, q1, q2);

		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 1u);
		CHECK(layered_ntk.num_layers() == 2u);
		CHECK(layered_ntk.layer(0).size() == 3u);
		CHECK(layered_ntk.layer(1).size() == 2u);
	}
	SECTION("Two layer of gates") {
		network.create_op(gate_lib::h, q2);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q2, q1);
		network.create_op(gate_lib::h, q0);

		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 2u);
		CHECK(layered_ntk.num_layers() == 3u);
		CHECK(layered_ntk.layer(0).size() == 3u);
		CHECK(layered_ntk.layer(1).size() == 2u);
		CHECK(layered_ntk.layer(2).size() == 2u);
	}
	SECTION("All outputs are in the last layer") {
		wire::id const q3 = network.create_qubit();
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q0, q3);
		network.create_op(gate_lib::h, q3);

		layers_view layered_ntk(network);
		CHECK(layered_ntk.depth() == 4u);
		CHECK(layered_ntk.num_layers() == 5u);
		CHECK(layered_ntk.layer(0).size() == 4u);
		CHECK(layered_ntk.layer(1).size() == 1u);
		CHECK(layered_ntk.layer(2).size() == 1u);
		CHECK(layered_ntk.layer(3).size() == 1u);
		CHECK(layered_ntk.layer(4).size() == 1u);
	}
}