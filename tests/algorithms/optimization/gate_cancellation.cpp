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
		CHECK(opt_network.size() == 5);
		CHECK(opt_network.num_qubits() == 2);
	}
	SECTION("Two qubit gates") {
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q0);
		auto opt_network = gate_cancellation(network);
		CHECK(opt_network.size() == 5);
		CHECK(opt_network.num_qubits() == 2);
	}
}
