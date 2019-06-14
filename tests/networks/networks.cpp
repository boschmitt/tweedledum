/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/gate_base.hpp>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Common functionality for all networks", "[networks][template]",
                           (gg_network, netlist), (io3_gate, mcmt_gate))
{
	TestType network;
	SECTION("An empty network")
	{
		CHECK(network.size() == 0);
		CHECK(network.num_qubits() == 0);
		CHECK(network.num_cbits() == 0);
		CHECK(network.num_gates() == 0);
	}
	SECTION("Simple network with 2 qubits and 2 cbits")
	{
		auto q0 = network.add_qubit("q0");
		auto c0 = network.add_cbit("c0");
		auto q1 = network.add_qubit();
		auto c1 = network.add_cbit();
		CHECK(q0 == 0);
		CHECK(c0 == 1);
		CHECK(q1 == 2);
		CHECK(c1 == 3);

		CHECK(q0.is_qubit());
		CHECK(q1.is_qubit());
		CHECK_FALSE(c0.is_qubit());
		CHECK_FALSE(c1.is_qubit());

		CHECK(network.size() == 8);
		CHECK(network.num_qubits() == 2);
		CHECK(network.num_cbits() == 2);
		SECTION("Add a hadamard gate")
		{
			auto node = network.add_gate(gate::hadamard, "q0");
			CHECK(network.size() == 9);
			CHECK(network.num_gates() == 1);
			CHECK(node.gate.target() == 0);
		}
		SECTION("Add a cnot gate")
		{
			auto node = network.add_gate(gate::cx, q1, q0);
			CHECK(network.size() == 9);
			CHECK(network.num_gates() == 1);
			CHECK(node.gate.control() == q1);
			CHECK(node.gate.target() == q0);
		}
		SECTION("Add a cz gate with negative control")
		{
			auto node = network.add_gate(gate::cz, !q1, q0);
			CHECK(network.size() == 9);
			CHECK(network.num_gates() == 1);
			CHECK(node.gate.control() == !q1);
			CHECK(node.gate.control().is_complemented());
			CHECK(node.gate.control().index() == q1.index());
			CHECK(node.gate.target() == q0);
		}
	}
}