/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/sabre_map.hpp"

#include "tweedledum/algorithms/verification/map_verify.hpp"
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/gates/w2_op.hpp"
#include "tweedledum/gates/w3_op.hpp"
#include "tweedledum/gates/wn32_op.hpp"
#include "tweedledum/networks/mapped_dag.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Test for Sabre mapper", "[sabre_map][template]", (op_dag),
                           (w2_op, w3_op, wn32_op))
{
	TestType network;

	SECTION("Simple circuit") {
		wire_id q0 = network.create_qubit();
		network.create_cbit();
		wire_id q1 = network.create_qubit();
		network.create_cbit();
		wire_id q2 = network.create_qubit();
		network.create_cbit();

		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q2, q0);

		device arch = device::path(network.num_qubits());
		mapped_dag mapped_ntk = sabre_map(network, arch);
		CHECK(map_verify(network, mapped_ntk));
	}

	wire_id q0 = network.create_qubit();
	network.create_cbit();
	wire_id q1 = network.create_qubit();
	wire_id q2 = network.create_qubit();
	network.create_cbit();
	wire_id q3 = network.create_qubit();
	network.create_cbit();

	SECTION("Extend ZDD mapper paper example") {
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q3, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q3);

		device arch = device::ring(network.num_qubits());
		mapped_dag mapped_ntk = sabre_map(network, arch);
		CHECK(map_verify(network, mapped_ntk));
	}
	SECTION("Extend ZDD mapper paper example #2") {
		network.create_qubit();
		auto q5 = network.create_qubit();

		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q5);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);

		device arch = device::ring(network.num_qubits());
		mapped_dag mapped_ntk = sabre_map(network, arch);
		CHECK(map_verify(network, mapped_ntk));
	}
	SECTION("Extend ZDD mapper paper example #3") {
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q3, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q3, q2);
		network.create_op(gate_lib::cx, q3, q1);
		network.create_op(gate_lib::cx, q3, q0);

		device arch = device::ring(network.num_qubits());
		mapped_dag mapped_ntk = sabre_map(network, arch);
		CHECK(map_verify(network, mapped_ntk));
	}
	SECTION("Extend ZDD mapper paper example #3.5") {
		wire_id q4 = network.create_qubit();
		wire_id q5 = network.create_qubit();
		wire_id q6 = network.create_qubit();
		wire_id q7 = network.create_qubit();
		wire_id q8 = network.create_qubit();
		wire_id q9 = network.create_qubit();

		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q5);
		network.create_op(gate_lib::cx, q9, q8);
		network.create_op(gate_lib::cx, q1, q5);
		network.create_op(gate_lib::cx, q4, q3);
		network.create_op(gate_lib::cx, q8, q7);
		network.create_op(gate_lib::cx, q6, q8);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q2, q5);
		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);

		device arch = device::ring(network.num_qubits());
		sabre_map_params ps;
		mapped_dag mapped_ntk = sabre_map(network, arch, ps);
		CHECK(map_verify(network, mapped_ntk));
	}
	SECTION("Extend ZDD mapper paper #4") {
		wire_id q4 = network.create_qubit();
		wire_id q5 = network.create_qubit();
		wire_id q6 = network.create_qubit();
		wire_id q7 = network.create_qubit();

		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q1, q3);
		network.create_op(gate_lib::cx, q4, q5);
		network.create_op(gate_lib::cx, q5, q6);
		network.create_op(gate_lib::cx, q5, q7);

		device arch = device::ring(network.num_qubits());
		sabre_map_params ps;
		auto mapped_ntk = sabre_map(network, arch, ps);
		CHECK(map_verify(network, mapped_ntk));
	}
	SECTION("Test for ZDD mapper") {
		wire_id q4 = network.create_qubit();
		wire_id q5 = network.create_qubit();

		network.create_op(gate_lib::cx, q0, q2);
		network.create_op(gate_lib::cx, q2, q1);
		network.create_op(gate_lib::cx, q0, q4);
		network.create_op(gate_lib::cx, q3, q0);
		network.create_op(gate_lib::cx, q0, q5);

		device arch = device::ring(network.num_qubits());
		mapped_dag mapped_ntk = sabre_map(network, arch);
		CHECK(map_verify(network, mapped_ntk));
	}
	SECTION("Test two consecutive swaps mapper") {
		wire_id q4 = network.create_qubit();

		network.create_op(gate_lib::cx, q0, q1);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q2, q3);
		network.create_op(gate_lib::cx, q3, q4);
		network.create_op(gate_lib::cx, q0, q4);

		device arch = device::path(network.num_qubits());
		sabre_map_params ps;
		mapped_dag mapped_ntk = sabre_map(network, arch, ps);
		CHECK(map_verify(network, mapped_ntk));
	}
}
