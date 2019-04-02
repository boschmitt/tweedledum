/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/gate_base.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
// #include <tweedledum/gates/q_gate.hpp>
#include <tweedledum/networks/qubit.hpp>

TEMPLATE_TEST_CASE("Common functionality for all gate kinds", "[gates][template]",
                   tweedledum::mcst_gate, tweedledum::mcmt_gate)
{
	using namespace tweedledum;

	qubit_id q0(0);
	qubit_id q1(1);
	qubit_id q2(2);

	SECTION("Create a hadamard gate") {
		TestType gate(gate::hadamard, q0);
		CHECK(gate.operation() == gate_set::hadamard);
		CHECK(gate.num_controls() == 0u);
		CHECK(gate.num_targets() == 1u);
		CHECK(gate.control() == qid_invalid);
		CHECK(gate.target() == q0);
	}
	SECTION("Create controlled gate") {
		TestType gate(gate::cx, q0, q1);
		CHECK(gate.operation() == gate_set::cx);
		CHECK(gate.num_controls() == 1u);
		CHECK(gate.num_targets() == 1u);
		CHECK(gate.control() == q0);
		CHECK(gate.target() == q1);
	}
	SECTION("Create controlled gate using vectors") {
		std::vector<qubit_id> controls = {q0};
		std::vector<qubit_id> targets = {q1};
		TestType gate(gate::cx, controls, targets);
		CHECK(gate.operation() == gate_set::cx);
		CHECK(gate.num_controls() == 1u);
		CHECK(gate.num_targets() == 1u);
		CHECK(gate.control() == q0);
		CHECK(gate.target() == q1);
	}
	SECTION("Create multiple controlled gate") {
		std::vector<qubit_id> controls = {q0, q1};
		std::vector<qubit_id> targets = {q2};
		TestType gate(gate::mcx, controls, targets);
		CHECK(gate.operation() == gate_set::mcx);
		CHECK(gate.num_controls() == 2u);
		CHECK(gate.num_targets() == 1u);
		CHECK(gate.control() == qid_invalid);
		CHECK(gate.target() == q2);
	}
}