/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/optimization/gate_cancellation.hpp"

#include "tweedledum/algorithms/verification/unitary_verify.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"

#include <catch.hpp>
#include <vector>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Simple gate cancellations", "[gate_cancellation][optmization]",
                           (op_dag), (w3_op, wn32_op))
{
	TestType network;
	SECTION("Single qubit gates") {
		wire_id q0 = network.create_qubit("q0");
		wire_id q1 = network.create_qubit();
		network.create_op(gate_lib::h, q0);
		network.create_op(gate_lib::h, q0);
		network.create_op(gate_lib::h, q1);
		network.create_op(gate_lib::t, q1);
		network.create_op(gate_lib::tdg, q1);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("1") {
		wire_id q0 = network.create_qubit("q0");
		wire_id q1 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q0);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("2") {
		wire_id q0 = network.create_qubit("q0");
		wire_id q1 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::t, q0);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q0);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 2);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("3") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q0, q2);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("4") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q0, q2);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("5") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::cx, q2, q1);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q2, q1);
		network.create_op(gate_lib::cx, q0, q1);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("6") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::cx, q2, q1);
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q2, q1);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q0, q2);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 4);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("Multiple qubit gates") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::ncz, q0, q1, q2);
		network.create_op(gate_lib::ncx, q0, q1, q2);
		network.create_op(gate_lib::ncx, q0, q1, q2);
		network.create_op(gate_lib::ncz, q0, q1, q2);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("Multiple qubit gates") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::ncx, q0, q1, q2);
		network.create_op(gate_lib::cx, q0, q1);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 3);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("Multiple qubit gates") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::ncx, q0, q1, q2);
		network.create_op(gate_lib::cx, q0, q2);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("Multiple qubit gates") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		wire_id q3 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::ncx, q1, q2, q3);
		network.create_op(gate_lib::cx, q0, q1);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 3);
		CHECK(unitary_verify(network, optmized));
	}
	SECTION("Multiple qubit gates") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		wire_id q3 = network.create_qubit();
		wire_id q4 = network.create_qubit();
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::ncx, q2, q3, q4);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::ncx, q2, q3, q4);
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 5);
		CHECK(unitary_verify(network, optmized));
	}
}

TEMPLATE_PRODUCT_TEST_CASE("Even Sequences", "[gate_cancellation][optmization]", (op_dag),
                           (wn32_op, w3_op))
{
	TestType network;
	SECTION("Even sequece of hadamards") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.create_op(gate_lib::h, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
	}
	SECTION("Even sequece of pauli-x") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.create_op(gate_lib::x, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
	}
	SECTION("Even sequece of pauli-z") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.create_op(gate_lib::z, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
	}
	SECTION("Even sequece of pauli-y") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.create_op(gate_lib::y, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
	}
	SECTION("Even sequece of cx") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.create_op(gate_lib::cx, q0, q1);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
	}
	SECTION("Even sequece of cx") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.create_op(gate_lib::cx, q0, q1);
			network.create_op(gate_lib::cx, q2, q1);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 0);
	}
}

TEMPLATE_PRODUCT_TEST_CASE("Odd Sequences", "[gate_cancellation][optmization]", (op_dag),
                           (wn32_op, w3_op))
{
	TestType network;
	SECTION("Odd sequece of hadamards") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.create_op(gate_lib::h, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
	}
	SECTION("Odd sequece of pauli-x") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.create_op(gate_lib::x, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
	}
	SECTION("Odd sequece of pauli-z") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.create_op(gate_lib::z, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
	}
	SECTION("Odd sequece of pauli-y") {
		wire_id q0 = network.create_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.create_op(gate_lib::y, q0);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
	}
	SECTION("Odd sequece of cx") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.create_op(gate_lib::cx, q0, q1);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 1);
	}
	SECTION("Odd sequece of cx") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.create_op(gate_lib::cx, q0, q1);
			network.create_op(gate_lib::cx, q2, q1);
		}
		auto optmized = gate_cancellation(network);
		CHECK(optmized.num_operations() == 2);
	}
}
