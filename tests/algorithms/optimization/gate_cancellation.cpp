/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <string>
#include <tweedledum/algorithms/optimization/gate_cancellation.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <vector>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Simple gate cancellations", "[gate_cancellation][template]",
                           (gg_network), (mcmt_gate, io3_gate))
{
	TestType network;
	SECTION("Single qubit gates") {
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::hadamard, q1);
		network.add_gate(gate::t, q1);
		network.add_gate(gate::t_dagger, q1);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
		CHECK(opt_network.num_qubits() == 2);
	}
	SECTION("1") {
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q0);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
		CHECK(opt_network.num_qubits() == 2);
	}
	SECTION("2") {
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::t, q0);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q0);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 2);
		CHECK(opt_network.num_qubits() == 2);
	}
	SECTION("3") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::cx, q1, q0);
		network.add_gate(gate::cx, q1, q0);
		network.add_gate(gate::cx, q0, q2);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.size() == 6);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("4") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q0, q2);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.size() == 6);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("5") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::cx, q2, q1);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q2, q1);
		network.add_gate(gate::cx, q0, q1);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.size() == 6);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("6") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::cx, q2, q1);
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q2, q1);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q0, q2);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.size() == 10);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 4);
	}
	SECTION("Multiple qubit gates") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::mcz, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));
		network.add_gate(gate::mcx, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));
		network.add_gate(gate::mcx, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));
		network.add_gate(gate::mcz, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("Multiple qubit gates") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::mcx, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));
		network.add_gate(gate::cx, q0, q1);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 3);
	}
	SECTION("Multiple qubit gates") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::mcx, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));
		network.add_gate(gate::cx, q0, q2);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_qubits() == 3);
		CHECK(opt_network.num_gates() == 1);
	}
	SECTION("Multiple qubit gates") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		auto q3 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::mcx, std::vector<io_id>({q1, q2}), std::vector<io_id>({q3}));
		network.add_gate(gate::cx, q0, q1);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_qubits() == 4);
		CHECK(opt_network.num_gates() == 3);
	}
	SECTION("Multiple qubit gates") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		auto q3 = network.add_qubit();
		auto q4 = network.add_qubit();
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::mcx, std::vector<io_id>({q2, q3}), std::vector<io_id>({q4}));
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::mcx, std::vector<io_id>({q2, q3}), std::vector<io_id>({q4}));
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_qubits() == 5);
		CHECK(opt_network.num_gates() == 5);
	}
}

TEMPLATE_PRODUCT_TEST_CASE("Even Sequences", "[gate_cancellation][template]",
                           (gg_network), (mcmt_gate, io3_gate))
{
	TestType network;
	SECTION("Even sequece of hadamards") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.add_gate(gate::hadamard, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("Even sequece of pauli-x") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.add_gate(gate::pauli_x, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("Even sequece of pauli-z") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.add_gate(gate::pauli_z, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("Even sequece of pauli-y") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.add_gate(gate::pauli_y, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("Even sequece of cx") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.add_gate(gate::cx, q0, q1);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 0);
	}
	SECTION("Even sequece of cx") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		for (uint32_t i = 0; i < 1024; ++i) {
			network.add_gate(gate::cx, q0, q1);
			network.add_gate(gate::cx, q2, q1);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 0);
	}
}

TEMPLATE_PRODUCT_TEST_CASE("Odd Sequences", "[gate_cancellation][template]",
                           (gg_network), (mcmt_gate, io3_gate))
{
	TestType network;
	SECTION("Odd sequece of hadamards") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.add_gate(gate::hadamard, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
	}
	SECTION("Odd sequece of pauli-x") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.add_gate(gate::pauli_x, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
	}
	SECTION("Odd sequece of pauli-z") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.add_gate(gate::pauli_z, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
	}
	SECTION("Odd sequece of pauli-y") {
		auto q0 = network.add_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.add_gate(gate::pauli_y, q0);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
	}
	SECTION("Odd sequece of cx") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.add_gate(gate::cx, q0, q1);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 1);
	}
	SECTION("Odd sequece of cx") {
		auto q0 = network.add_qubit();
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		for (uint32_t i = 0; i < 1023; ++i) {
			network.add_gate(gate::cx, q0, q1);
			network.add_gate(gate::cx, q2, q1);
		}
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.num_gates() == 2);
	}
}
