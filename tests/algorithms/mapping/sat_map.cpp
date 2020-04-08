/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/sat_map.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/mapped_dag.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Test for SAT mapper", "[sat_map][mapping]", (netlist, op_dag),
                           (w3_op, wn32_op))
{
	TestType network;
	SECTION("Empty network") {
		device device = device::path(network.num_qubits());
		auto mapped_ntk = sat_map(network, device);
		CHECK(mapped_ntk.size() == 0u);
		CHECK(mapped_ntk.num_wires() == 0u);
		CHECK(mapped_ntk.num_qubits() == 0u);
		CHECK(mapped_ntk.num_cbits() == 0u);
		CHECK(mapped_ntk.num_operations() == 0u);
	}
	SECTION("Network with no operations") {
		network.create_qubit();
		network.create_cbit();
		network.create_qubit();
		network.create_cbit();
		network.create_qubit();
		network.create_cbit();
		device device = device::path(network.num_qubits());
		auto mapped_ntk = sat_map(network, device);
		CHECK(mapped_ntk.size() == network.size());
		CHECK(mapped_ntk.num_wires() == network.num_wires());
		CHECK(mapped_ntk.num_qubits() == network.num_qubits());
		CHECK(mapped_ntk.num_cbits() == network.num_cbits());
		CHECK(mapped_ntk.num_operations() == 0u);
	}
	SECTION("Simple circuit (SAT)") {
		wire::id q0 = network.create_qubit();
		network.create_cbit();
		wire::id q1 = network.create_qubit();
		network.create_cbit();
		wire::id q2 = network.create_qubit();
		network.create_cbit();

		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q2, q0);

		device device = device::path(network.num_qubits());
		auto mapped_ntk = sat_map(network, device);
		CHECK(mapped_ntk.num_operations() == 2u);
	}
	SECTION("Simple circuit (UNSAT)") {
		wire::id q0 = network.create_qubit();
		network.create_cbit();
		wire::id q1 = network.create_qubit();
		network.create_cbit();
		wire::id q2 = network.create_qubit();
		network.create_cbit();

		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q2, q0);

		device device = device::path(network.num_qubits());
		auto mapped_ntk = sat_map(network, device);
		CHECK(mapped_ntk.num_operations() == 0u);
	}
}
