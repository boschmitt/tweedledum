/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/networks/unitary.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Basic unitary functionality", "[unitary][networks]")
{
	SECTION("An empty unitary") {
		unitary u;
		CHECK(u.num_wires() == 0u);
		CHECK(u.num_qubits() == 0u);
	}
	SECTION("One-qubit unitary") {
		unitary u0(1u);
		CHECK(u0.num_wires() == 1u);
		CHECK(u0.num_qubits() == 1u);

		unitary u1;
		u1.create_qubit();
		CHECK(u1.num_wires() == 1u);
		CHECK(u1.num_qubits() == 1u);
		CHECK(u0.is_apprx_equal(u1));
		CHECK(u1.is_apprx_equal(u0));
	}
	SECTION("Two-qubit unitary") {
		unitary u0(2u);
		CHECK(u0.num_wires() == 2u);
		CHECK(u0.num_qubits() == 2u);

		unitary u1;
		u1.create_qubit();
		u1.create_qubit();
		CHECK(u1.num_wires() == 2u);
		CHECK(u1.num_qubits() == 2u);
		CHECK(u0.is_apprx_equal(u1));
		CHECK(u1.is_apprx_equal(u0));
	}
	SECTION("Two-qubit unitary") {
		unitary u0;
		wire::id q0 = u0.create_qubit();
		u0.create_op(gate_lib::x, q0);
		u0.create_qubit();

		unitary u1(2u);
		u1.create_op(gate_lib::x, q0);
		CHECK(u0.is_apprx_equal(u1));
		CHECK(u1.is_apprx_equal(u0));
	}
}