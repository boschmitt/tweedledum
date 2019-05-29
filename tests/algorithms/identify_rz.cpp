/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/identify_rz.hpp>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/angle.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Try to identify Rz", "[identify_rz][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	TestType network;
	auto qubit = network.add_qubit();
	SECTION("T gate")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, angles::pi_quarter), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::t));
	}
	SECTION("S gate (phase)")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, angles::pi_half), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::phase));
	}
	SECTION("Pauli Z gate")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, angles::pi), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::pauli_z));
	}
	SECTION("T† gate (Conjugate transpose)- negative rotation")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, -angles::pi_quarter), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::t_dagger));
	}
	SECTION("S† gate (Conjugate transpose) - negative rotation")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, -angles::pi_half), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::phase_dagger));
	}
	SECTION("T† gate (Conjugate transpose)- positive rotation")
	{
		angle rotation_angle(7, 4);
		network.add_gate(gate_base(gate_lib::rotation_z, rotation_angle), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::t_dagger));
	}
	SECTION("S† gate (Conjugate transpose) - positive rotation")
	{
		angle rotation_angle(3, 2);
		network.add_gate(gate_base(gate_lib::rotation_z, rotation_angle), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 1);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::phase_dagger));
	}
	SECTION("P + T")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, angles::pi_quarter + angles::pi_half), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 2);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::phase));
		node = result.vertex(2u);
		CHECK(node.gate.is(gate_lib::t));
	}
	SECTION("Z + T")
	{
		network.add_gate(gate_base(gate_lib::rotation_z, angles::pi_quarter + angles::pi), qubit);
		TestType result = identify_rz(network);
		CHECK(result.num_qubits() == 1);
		CHECK(result.num_gates() == 2);
		auto& node = result.vertex(1u);
		CHECK(node.gate.is(gate_lib::pauli_z));
		node = result.vertex(2u);
		CHECK(node.gate.is(gate_lib::t));
	}
}
