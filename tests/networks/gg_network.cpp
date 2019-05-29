/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <string>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <vector>

TEST_CASE("Gate graph network simple constructors", "[gg_network]")
{
	using namespace tweedledum;
	SECTION("No gates") {
		gg_network<io3_gate> network;
		CHECK(network.size() == 0);
		network.add_qubit("q0");
		network.add_qubit();
		CHECK(network.size() == 4);
		CHECK(network.num_qubits() == 2);
	}
	SECTION("One gate") {
		gg_network<io3_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		CHECK(network.size() == 5);
		CHECK(network.num_qubits() == 2);
	}
	SECTION("Two gates") {
		gg_network<io3_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::cx, q0, q1);
		CHECK(network.size() == 6);
		CHECK(network.num_qubits() == 2);
	}
	SECTION("Two gates, negative control") {
		gg_network<io3_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::cx, !q0, q1);
		CHECK(network.size() == 6);
		CHECK(network.num_qubits() == 2);
	}
	SECTION("One Tofolli gate, negative control") {
		gg_network<io3_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::mcx, std::vector({!q0, q1}), std::vector({q2}));
		CHECK(network.size() == 7);
		CHECK(network.num_qubits() == 3);
	}
}
