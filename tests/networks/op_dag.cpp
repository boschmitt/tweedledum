/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/networks/op_dag.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/wire.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Operations DAG 'foreach_input' iterator", "[op_dag][template]",
                           (op_dag), (w3_op, wn32_op))
{
	using node_type = typename TestType::node_type;
	TestType network;
	SECTION("Input iterator") {
		network.create_qubit();
		network.create_qubit();
		network.create_qubit();
		network.create_qubit();
		uint32_t i = 0u;
		network.foreach_input([&] (node_id const id) {
			CHECK(id == i);
			++i;
		});
		i = 0u;
		network.foreach_input([&] (node_type const& node, node_id const id) {
			CHECK(node.op.is(gate_ids::input));
			CHECK(id == i);
			++i;
		});
		i = 0u;
		network.foreach_input([&] (node_type const& node) {
			CHECK(node.op.is(gate_ids::input));
		});
	}
	SECTION("Output iterator") {
		wire::id q0 = network.create_qubit();
		wire::id q1 = network.create_qubit();
		wire::id q2 = network.create_qubit();
		uint32_t i = 0u;
		network.foreach_output([&] (node_id const id) {
			CHECK(id == i);
			++i;
		});
		i = 0u;
		network.foreach_output([&] (node_type const& node, node_id const id) {
			CHECK(node.op.is(gate_ids::input));
			CHECK(id == i);
			++i;
		});
		i = 0u;
		network.foreach_output([&] (node_type const& node) {
			CHECK(node.op.is(gate_ids::input));
		});

		node_id n = network.create_op(gate_lib::ncx, q0, q1, q2);
		network.foreach_output([&] (node_type const& node, node_id const id) {
			CHECK(node.op.is(gate_ids::ncx));
			CHECK(id == n);
		});
	}
}

TEMPLATE_PRODUCT_TEST_CASE("Operations 'foreach_output' iterator", "[op_dag][template]",
                           (op_dag), (w3_op, wn32_op))
{
	using node_type = typename TestType::node_type;
	TestType network;
	wire::id q0 = network.create_qubit();
	wire::id q1 = network.create_qubit();
	wire::id q2 = network.create_qubit();
	SECTION("No operations") {
		uint32_t i = 0u;
		network.foreach_output([&] (node_id const id) {
			CHECK(id == i);
			++i;
		});
		i = 0u;
		network.foreach_output([&] (node_type const& node, node_id const id) {
			CHECK(node.op.is(gate_ids::input));
			CHECK(id == i);
			++i;
		});
		i = 0u;
		network.foreach_output([&] (node_type const& node) {
			CHECK(node.op.is(gate_ids::input));
		});
	}
	SECTION("One operation") {
		node_id n = network.create_op(gate_lib::ncx, q0, q1, q2);
		network.foreach_output([&] (node_type const& node, node_id const id) {
			CHECK(node.op.is(gate_ids::ncx));
			CHECK(id == n);
		});
	}
	SECTION("One operation") {
		network.create_op(gate_lib::cx, q1, q0);
		node_id n1 = network.create_op(gate_lib::cx, q1, q2);
		node_id n2 = network.create_op(gate_lib::cx, q2, q0);

		uint32_t i = 0;
		std::array<node_id, 3> node_ids = {n2, n1 , n2};
		network.foreach_output([&] (node_type const& node, node_id const id) {
			CHECK(node.op.is(gate_ids::cx));
			CHECK(id == node_ids.at(i));
			++i;
		});
	}
}