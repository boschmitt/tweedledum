/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch.hpp>
#include <tweedledum/networks/gates/gate_kinds.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>

TEST_CASE("Check dependency among single qubit gates", "qc_gate")
{
	using namespace tweedledum;
	GIVEN("two single qubit gates") {
		qc_gate hadamard(gate_kinds_t::hadamard, 0);
		qc_gate pauli_z(gate_kinds_t::pauli_z, 0);
		qc_gate pauli_x(gate_kinds_t::pauli_x, 0);
		qc_gate t(gate_kinds_t::t, 0);
		qc_gate t_dagger(gate_kinds_t::t_dagger, 0);
		qc_gate phase(gate_kinds_t::phase, 0);
		qc_gate phase_dagger(gate_kinds_t::phase_dagger, 0);
		qc_gate rotation_x(gate_kinds_t::rotation_x, 0);
		qc_gate rotation_z(gate_kinds_t::rotation_z, 0);
		WHEN("they are both Rz") {
			THEN("they are independent") {
				CHECK_FALSE(pauli_z.is_dependent(t));
				CHECK_FALSE(pauli_z.is_dependent(t_dagger));
				CHECK_FALSE(pauli_z.is_dependent(phase));
				CHECK_FALSE(pauli_z.is_dependent(phase_dagger));
				CHECK_FALSE(pauli_z.is_dependent(rotation_z));

				CHECK_FALSE(t.is_dependent(pauli_z));
				CHECK_FALSE(t.is_dependent(t_dagger));
				CHECK_FALSE(t.is_dependent(phase));
				CHECK_FALSE(t.is_dependent(phase_dagger));
				CHECK_FALSE(t.is_dependent(rotation_z));

				CHECK_FALSE(t_dagger.is_dependent(pauli_z));
				CHECK_FALSE(t_dagger.is_dependent(t));
				CHECK_FALSE(t_dagger.is_dependent(phase));
				CHECK_FALSE(t_dagger.is_dependent(phase_dagger));
				CHECK_FALSE(t_dagger.is_dependent(rotation_z));

				CHECK_FALSE(phase.is_dependent(pauli_z));
				CHECK_FALSE(phase.is_dependent(t));
				CHECK_FALSE(phase.is_dependent(t_dagger));
				CHECK_FALSE(phase.is_dependent(phase_dagger));
				CHECK_FALSE(phase.is_dependent(rotation_z));

				CHECK_FALSE(phase_dagger.is_dependent(pauli_z));
				CHECK_FALSE(phase_dagger.is_dependent(t));
				CHECK_FALSE(phase_dagger.is_dependent(t_dagger));
				CHECK_FALSE(phase_dagger.is_dependent(phase));
				CHECK_FALSE(phase_dagger.is_dependent(rotation_z));

				CHECK_FALSE(rotation_z.is_dependent(pauli_z));
				CHECK_FALSE(rotation_z.is_dependent(t));
				CHECK_FALSE(rotation_z.is_dependent(t_dagger));
				CHECK_FALSE(rotation_z.is_dependent(phase));
				CHECK_FALSE(rotation_z.is_dependent(phase_dagger));
			}
		}
		WHEN("they are both Rx") {
			THEN("they are independent") {
				CHECK_FALSE(rotation_x.is_dependent(pauli_x));
				CHECK_FALSE(pauli_x.is_dependent(rotation_x));
			}
		}
		WHEN("one is hadamard and the other a rotation") {
			THEN("they are dependent") {
				CHECK(hadamard.is_dependent(pauli_z));
				CHECK(hadamard.is_dependent(pauli_x));
				CHECK(hadamard.is_dependent(t));
				CHECK(hadamard.is_dependent(t_dagger));
				CHECK(hadamard.is_dependent(phase));
				CHECK(hadamard.is_dependent(phase_dagger));
				CHECK(hadamard.is_dependent(rotation_x));
				CHECK(hadamard.is_dependent(rotation_z));

				CHECK(pauli_z.is_dependent(hadamard));
				CHECK(pauli_x.is_dependent(hadamard));
				CHECK(t.is_dependent(hadamard));
				CHECK(t_dagger.is_dependent(hadamard));
				CHECK(phase.is_dependent(hadamard));
				CHECK(phase_dagger.is_dependent(hadamard));
				CHECK(rotation_x.is_dependent(hadamard));
				CHECK(rotation_z.is_dependent(hadamard));
			}
		}
		WHEN("one is Rz and the Rx") {
			THEN("they are dependent") {
				CHECK(pauli_z.is_dependent(rotation_x));
				CHECK(t.is_dependent(rotation_x));
				CHECK(t_dagger.is_dependent(rotation_x));
				CHECK(phase.is_dependent(rotation_x));
				CHECK(phase_dagger.is_dependent(rotation_x));
				CHECK(rotation_z.is_dependent(rotation_x));

				CHECK(pauli_z.is_dependent(pauli_x));
				CHECK(t.is_dependent(pauli_x));
				CHECK(t_dagger.is_dependent(pauli_x));
				CHECK(phase.is_dependent(pauli_x));
				CHECK(phase_dagger.is_dependent(pauli_x));
				CHECK(rotation_z.is_dependent(pauli_x));

				CHECK(rotation_x.is_dependent(pauli_z));
				CHECK(rotation_x.is_dependent(t));
				CHECK(rotation_x.is_dependent(t_dagger));
				CHECK(rotation_x.is_dependent(phase));
				CHECK(rotation_x.is_dependent(phase_dagger));
				CHECK(rotation_x.is_dependent(rotation_z));

				CHECK(pauli_x.is_dependent(pauli_z));
				CHECK(pauli_x.is_dependent(t));
				CHECK(pauli_x.is_dependent(t_dagger));
				CHECK(pauli_x.is_dependent(phase));
				CHECK(pauli_x.is_dependent(phase_dagger));
				CHECK(pauli_x.is_dependent(rotation_z));
			}
		}
	}
}

TEST_CASE("Check CX CX dependency", "qc_gate")
{
	using namespace tweedledum;
	GIVEN("two CX gates") {
		WHEN("the controls are equal and the targets are equal") {
			// 0 -@--@--
			// 1 -X--X--
			qc_gate g0(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate g1(gate_kinds_t::cx, /* target */ 1, 0);
			// 0 -X--X--
			// 1 -@--@--
			qc_gate g2(gate_kinds_t::cx, /* target */ 0, 1);
			qc_gate g3(gate_kinds_t::cx, /* target */ 0, 1);
			THEN( "they are independent" ) {
				CHECK_FALSE(g0.is_dependent(g1));
				CHECK_FALSE(g1.is_dependent(g0));
				CHECK_FALSE(g2.is_dependent(g3));
				CHECK_FALSE(g3.is_dependent(g2));
			}
		}
		WHEN("the controls are different and the targets are equal") {
			// 0 -@-----
			// 1 -X--X--
			// 2 ----@--
			qc_gate g0(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate g1(gate_kinds_t::cx, /* target */ 1, 2);
			THEN( "they are independent" ) {
				CHECK_FALSE(g0.is_dependent(g1));
				CHECK_FALSE(g1.is_dependent(g0));
			}
		}
		WHEN("the controls are equal and the targets are different") {
			// 0 -@--@--
			// 1 -X--|--
			// 2 ----X--
			qc_gate g0(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate g1(gate_kinds_t::cx, /* target */ 2, 0);
			THEN( "they are independent" ) {
				CHECK_FALSE(g0.is_dependent(g1));
				CHECK_FALSE(g1.is_dependent(g0));
			}
		}
		WHEN("the control of one is target of the other") {
			// 0 -@-----
			// 1 -X--@--
			// 2 ----X--
			qc_gate g0(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate g1(gate_kinds_t::cx, /* target */ 2, 1);
			// 0 -@--X--
			// 1 -X--@--
			qc_gate g2(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate g3(gate_kinds_t::cx, /* target */ 0, 1);
			THEN( "they are dependent" ) {
				CHECK(g0.is_dependent(g1));
				CHECK(g1.is_dependent(g0));
				CHECK(g2.is_dependent(g3));
				CHECK(g3.is_dependent(g2));
			}
		}
	}
}

TEST_CASE("Check CX Rx dependency", "qc_gate")
{
	using namespace tweedledum;
	GIVEN("two gates: CX and X") {
		WHEN("Rx acts on the control of CX") {
			// 0 -@--Rx-
			// 1 -X-----
			qc_gate cx(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate pauli_x(gate_kinds_t::pauli_x, 0);
			qc_gate rx(gate_kinds_t::rotation_x, 0);
			THEN("they are dependent") {
				CHECK(cx.is_dependent(pauli_x));
				CHECK(pauli_x.is_dependent(cx));
				CHECK(cx.is_dependent(rx));
				CHECK(rx.is_dependent(cx));
			}
		}
		WHEN("Rx acts on the target of CX") {
			// 0 -@-----
			// 1 -X--Rx-
			qc_gate cx(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate pauli_x(gate_kinds_t::pauli_x, 1);
			qc_gate rx(gate_kinds_t::rotation_x, 1);
			THEN("they are dependent") {
				CHECK_FALSE(cx.is_dependent(pauli_x));
				CHECK_FALSE(pauli_x.is_dependent(cx));
				CHECK_FALSE(cx.is_dependent(rx));
				CHECK_FALSE(rx.is_dependent(cx));
			}
		}
	}
}

TEST_CASE("Check CX Rz dependency", "qc_gate")
{
	using namespace tweedledum;
	GIVEN("two gates: CX and Rz") {
		WHEN("Rz acts on the control of CX") {
			// 0 -@--Rz-
			// 1 -X-----
			qc_gate cx(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate t(gate_kinds_t::t, 0);
			qc_gate t_dagger(gate_kinds_t::t_dagger, 0);
			qc_gate phase(gate_kinds_t::phase, 0);
			qc_gate phase_dagger(gate_kinds_t::phase_dagger, 0);
			qc_gate rotation_z(gate_kinds_t::rotation_z, 0);
			qc_gate pauli_z(gate_kinds_t::pauli_z, 0);
			THEN("they are independent") {
				CHECK_FALSE(cx.is_dependent(t));
				CHECK_FALSE(t.is_dependent(cx));
				CHECK_FALSE(cx.is_dependent(t_dagger));
				CHECK_FALSE(t_dagger.is_dependent(cx));
				CHECK_FALSE(cx.is_dependent(phase));
				CHECK_FALSE(phase.is_dependent(cx));
				CHECK_FALSE(cx.is_dependent(phase_dagger));
				CHECK_FALSE(phase_dagger.is_dependent(cx));
				CHECK_FALSE(cx.is_dependent(rotation_z));
				CHECK_FALSE(rotation_z.is_dependent(cx));
				CHECK_FALSE(cx.is_dependent(pauli_z));
				CHECK_FALSE(pauli_z.is_dependent(cx));
			}
		}
		WHEN("Rz acts on the target of CX") {
			// 0 -@-----
			// 1 -X--Rz
			qc_gate cx(gate_kinds_t::cx, /* target */ 1, 0);
			qc_gate t(gate_kinds_t::t, 1);
			qc_gate t_dagger(gate_kinds_t::t_dagger, 1);
			qc_gate phase(gate_kinds_t::phase, 1);
			qc_gate phase_dagger(gate_kinds_t::phase_dagger, 1);
			qc_gate rotation_z(gate_kinds_t::rotation_z, 1);
			qc_gate pauli_z(gate_kinds_t::pauli_z, 1);
			THEN("they are dependent") {
				CHECK(cx.is_dependent(t));
				CHECK(t.is_dependent(cx));
				CHECK(cx.is_dependent(t_dagger));
				CHECK(t_dagger.is_dependent(cx));
				CHECK(cx.is_dependent(phase));
				CHECK(phase.is_dependent(cx));
				CHECK(cx.is_dependent(phase_dagger));
				CHECK(phase_dagger.is_dependent(cx));
				CHECK(cx.is_dependent(rotation_z));
				CHECK(rotation_z.is_dependent(cx));
				CHECK(cx.is_dependent(pauli_z));
				CHECK(pauli_z.is_dependent(cx));
			}
		}
	}
}
