/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/gates/gate.hpp"

#include "tweedledum/utils/angle.hpp"

#include <catch.hpp>
#include <vector>

TEST_CASE("Check correcteness meta gates", "[gate][meta]")
{
	using namespace tweedledum;
	SECTION("Input gate") {
		CHECK(gate_lib::input.id() == gate_ids::input);
		CHECK(gate_lib::input.is(gate_ids::input));
		CHECK_FALSE(gate_lib::input.is_one_qubit());
		CHECK(gate_lib::input.is_meta());
		CHECK_FALSE(gate_lib::input.is_two_qubit());
		CHECK_FALSE(gate_lib::input.is_r1());
		CHECK_FALSE(gate_lib::input.is_measurement());
	}
}

TEST_CASE("Check correcteness non-parameterasible gates", "[gate][non_param]")
{
	using namespace tweedledum;
	SECTION("Identity gate") {
		CHECK(gate_lib::i.id() == gate_ids::i);
		CHECK(gate_lib::i.is(gate_ids::i));
		CHECK(gate_lib::i.axis() == rot_axis::na);
		CHECK(gate_lib::i.is_one_qubit());
		CHECK_FALSE(gate_lib::i.is_meta());
		CHECK_FALSE(gate_lib::i.is_two_qubit());
		CHECK_FALSE(gate_lib::i.is_r1());
		CHECK_FALSE(gate_lib::i.is_measurement());
	}
	SECTION("Hadamard gate") {
		CHECK(gate_lib::h.id() == gate_ids::h);
		CHECK(gate_lib::h.is(gate_ids::h));
		CHECK(gate_lib::h.axis() == rot_axis::xy);
		CHECK(gate_lib::h.is_one_qubit());
		CHECK_FALSE(gate_lib::h.is_meta());
		CHECK_FALSE(gate_lib::h.is_two_qubit());
		CHECK_FALSE(gate_lib::h.is_r1());
		CHECK_FALSE(gate_lib::h.is_measurement());
	}
	SECTION("X (NOT) gate") {
		CHECK(gate_lib::x.id() == gate_ids::x);
		CHECK(gate_lib::x.is(gate_ids::x));
		CHECK(gate_lib::x.axis() == rot_axis::x);
		CHECK(gate_lib::x.is_one_qubit());
		CHECK_FALSE(gate_lib::x.is_meta());
		CHECK_FALSE(gate_lib::x.is_two_qubit());
		CHECK_FALSE(gate_lib::x.is_r1());
		CHECK_FALSE(gate_lib::x.is_measurement());
		CHECK(gate_lib::x.rotation_angle() == sym_angle::pi);
	}
	SECTION("Y gate") {
		CHECK(gate_lib::y.id() == gate_ids::y);
		CHECK(gate_lib::y.is(gate_ids::y));
		CHECK(gate_lib::y.axis() == rot_axis::y);
		CHECK(gate_lib::y.is_one_qubit());
		CHECK_FALSE(gate_lib::y.is_meta());
		CHECK_FALSE(gate_lib::y.is_two_qubit());
		CHECK_FALSE(gate_lib::y.is_r1());
		CHECK_FALSE(gate_lib::y.is_measurement());
		CHECK(gate_lib::y.rotation_angle() == sym_angle::pi);
	}
	SECTION("Z gate") {
		CHECK(gate_lib::z.id() == gate_ids::z);
		CHECK(gate_lib::z.is(gate_ids::z));
		CHECK(gate_lib::z.axis() == rot_axis::z);
		CHECK(gate_lib::z.is_one_qubit());
		CHECK_FALSE(gate_lib::z.is_meta());
		CHECK_FALSE(gate_lib::z.is_two_qubit());
		CHECK(gate_lib::z.is_r1());
		CHECK_FALSE(gate_lib::z.is_measurement());
		CHECK(gate_lib::z.rotation_angle() == sym_angle::pi);
	}
	SECTION("Phase (S) gate") {
		CHECK(gate_lib::s.id() == gate_ids::s);
		CHECK(gate_lib::s.is(gate_ids::s));
		CHECK(gate_lib::s.axis() == rot_axis::z);
		CHECK(gate_lib::s.is_one_qubit());
		CHECK_FALSE(gate_lib::s.is_meta());
		CHECK_FALSE(gate_lib::s.is_two_qubit());
		CHECK(gate_lib::s.is_r1());
		CHECK_FALSE(gate_lib::s.is_measurement());
		CHECK(gate_lib::s.rotation_angle() == sym_angle::pi_half);
	}
	SECTION("Phase (S) adjoint (daggert) gate") {
		CHECK(gate_lib::sdg.id() == gate_ids::sdg);
		CHECK(gate_lib::sdg.is(gate_ids::sdg));
		CHECK(gate_lib::sdg.axis() == rot_axis::z);
		CHECK(gate_lib::sdg.is_one_qubit());
		CHECK_FALSE(gate_lib::sdg.is_meta());
		CHECK_FALSE(gate_lib::sdg.is_two_qubit());
		CHECK(gate_lib::sdg.is_r1());
		CHECK_FALSE(gate_lib::sdg.is_measurement());
		CHECK(gate_lib::sdg.rotation_angle() == -sym_angle::pi_half);
	}
	SECTION("T gate") {
		CHECK(gate_lib::t.id() == gate_ids::t);
		CHECK(gate_lib::t.is(gate_ids::t));
		CHECK(gate_lib::t.axis() == rot_axis::z);
		CHECK(gate_lib::t.is_one_qubit());
		CHECK_FALSE(gate_lib::t.is_meta());
		CHECK_FALSE(gate_lib::t.is_two_qubit());
		CHECK(gate_lib::t.is_r1());
		CHECK_FALSE(gate_lib::t.is_measurement());
		CHECK(gate_lib::t.rotation_angle() == sym_angle::pi_quarter);
	}
	SECTION("T adjoint (daggert) gate") {
		CHECK(gate_lib::tdg.id() == gate_ids::tdg);
		CHECK(gate_lib::tdg.is(gate_ids::tdg));
		CHECK(gate_lib::tdg.axis() == rot_axis::z);
		CHECK(gate_lib::tdg.is_one_qubit());
		CHECK_FALSE(gate_lib::tdg.is_meta());
		CHECK_FALSE(gate_lib::tdg.is_two_qubit());
		CHECK(gate_lib::tdg.is_r1());
		CHECK_FALSE(gate_lib::tdg.is_measurement());
		CHECK(gate_lib::tdg.rotation_angle() == -sym_angle::pi_quarter);
	}
	SECTION("CX (CNOT) gate") {
		CHECK(gate_lib::cx.id() == gate_ids::cx);
		CHECK(gate_lib::cx.is(gate_ids::cx));
		CHECK(gate_lib::cx.axis() == rot_axis::x);
		CHECK_FALSE(gate_lib::cx.is_one_qubit());
		CHECK_FALSE(gate_lib::cx.is_meta());
		CHECK(gate_lib::cx.is_two_qubit());
		CHECK_FALSE(gate_lib::cx.is_r1());
		CHECK_FALSE(gate_lib::cx.is_measurement());
		CHECK(gate_lib::cx.rotation_angle() == sym_angle::pi);
	}
	SECTION("CY gate") {
		CHECK(gate_lib::cy.id() == gate_ids::cy);
		CHECK(gate_lib::cy.is(gate_ids::cy));
		CHECK(gate_lib::cy.axis() == rot_axis::y);
		CHECK_FALSE(gate_lib::cy.is_one_qubit());
		CHECK_FALSE(gate_lib::cy.is_meta());
		CHECK(gate_lib::cy.is_two_qubit());
		CHECK_FALSE(gate_lib::cy.is_r1());
		CHECK_FALSE(gate_lib::cy.is_measurement());
		CHECK(gate_lib::cy.rotation_angle() == sym_angle::pi);
	}
	SECTION("CZ gate") {
		CHECK(gate_lib::cz.id() == gate_ids::cz);
		CHECK(gate_lib::cz.is(gate_ids::cz));
		CHECK(gate_lib::cz.axis() == rot_axis::z);
		CHECK_FALSE(gate_lib::cz.is_one_qubit());
		CHECK_FALSE(gate_lib::cz.is_meta());
		CHECK(gate_lib::cz.is_two_qubit());
		CHECK(gate_lib::cz.is_r1());
		CHECK_FALSE(gate_lib::cz.is_measurement());
		CHECK(gate_lib::cz.rotation_angle() == sym_angle::pi);
	}
	SECTION("NCX (Toffoli) gate") {
		CHECK(gate_lib::ncx.id() == gate_ids::ncx);
		CHECK(gate_lib::ncx.is(gate_ids::ncx));
		CHECK(gate_lib::ncx.axis() == rot_axis::x);
		CHECK_FALSE(gate_lib::ncx.is_one_qubit());
		CHECK_FALSE(gate_lib::ncx.is_meta());
		CHECK_FALSE(gate_lib::ncx.is_two_qubit());
		CHECK_FALSE(gate_lib::ncx.is_r1());
		CHECK_FALSE(gate_lib::ncx.is_measurement());
		CHECK(gate_lib::ncx.rotation_angle() == sym_angle::pi);
	}
	SECTION("NCY gate") {
		CHECK(gate_lib::ncy.id() == gate_ids::ncy);
		CHECK(gate_lib::ncy.is(gate_ids::ncy));
		CHECK(gate_lib::ncy.axis() == rot_axis::y);
		CHECK_FALSE(gate_lib::ncy.is_one_qubit());
		CHECK_FALSE(gate_lib::ncy.is_meta());
		CHECK_FALSE(gate_lib::ncy.is_two_qubit());
		CHECK_FALSE(gate_lib::ncy.is_r1());
		CHECK_FALSE(gate_lib::ncy.is_measurement());
		CHECK(gate_lib::ncy.rotation_angle() == sym_angle::pi);
	}
	SECTION("NCZ gate") {
		CHECK(gate_lib::ncz.id() == gate_ids::ncz);
		CHECK(gate_lib::ncz.is(gate_ids::ncz));
		CHECK(gate_lib::ncz.axis() == rot_axis::z);
		CHECK_FALSE(gate_lib::ncz.is_one_qubit());
		CHECK_FALSE(gate_lib::ncz.is_meta());
		CHECK_FALSE(gate_lib::ncz.is_two_qubit());
		CHECK(gate_lib::ncz.is_r1());
		CHECK_FALSE(gate_lib::ncz.is_measurement());
		CHECK(gate_lib::ncz.rotation_angle() == sym_angle::pi);
	}
	SECTION("SWAP gate") {
		CHECK(gate_lib::swap.id() == gate_ids::swap);
		CHECK(gate_lib::swap.is(gate_ids::swap));
		CHECK(gate_lib::swap.axis() == rot_axis::na);
		CHECK_FALSE(gate_lib::swap.is_one_qubit());
		CHECK_FALSE(gate_lib::swap.is_meta());
		CHECK(gate_lib::swap.is_two_qubit());
		CHECK_FALSE(gate_lib::swap.is_r1());
		CHECK_FALSE(gate_lib::swap.is_measurement());
	}
}

TEST_CASE("Check correcteness parameterasible gates", "[gate][param]")
{
	using namespace tweedledum;
	std::vector<angle> commmon_angles = {sym_angle::zero, sym_angle::pi, sym_angle::pi_half,
	                                     sym_angle::pi_quarter};

	SECTION("R1") {
		for (angle const& a : commmon_angles) {
			gate r1 = gate_lib::r1(a);
			CHECK(r1.id() == gate_ids::r1);
			CHECK(r1.is(gate_ids::r1));
			CHECK(r1.axis() == rot_axis::z);
			CHECK(r1.is_one_qubit());
			CHECK_FALSE(r1.is_meta());
			CHECK_FALSE(r1.is_two_qubit());
			CHECK(r1.is_r1());
			CHECK_FALSE(r1.is_measurement());
			CHECK(r1.rotation_angle() == a);
		}
	}
	SECTION("Rx") {
		for (angle const& a : commmon_angles) {
			gate rx = gate_lib::rx(a);
			CHECK(rx.id() == gate_ids::rx);
			CHECK(rx.is(gate_ids::rx));
			CHECK(rx.axis() == rot_axis::x);
			CHECK(rx.is_one_qubit());
			CHECK_FALSE(rx.is_meta());
			CHECK_FALSE(rx.is_two_qubit());
			CHECK_FALSE(rx.is_r1());
			CHECK_FALSE(rx.is_measurement());
			CHECK(rx.rotation_angle() == a);
		}
	}
	SECTION("Ry") {
		for (angle const& a : commmon_angles) {
			gate ry = gate_lib::ry(a);
			CHECK(ry.id() == gate_ids::ry);
			CHECK(ry.is(gate_ids::ry));
			CHECK(ry.axis() == rot_axis::y);
			CHECK(ry.is_one_qubit());
			CHECK_FALSE(ry.is_meta());
			CHECK_FALSE(ry.is_two_qubit());
			CHECK_FALSE(ry.is_r1());
			CHECK_FALSE(ry.is_measurement());
			CHECK(ry.rotation_angle() == a);
		}
	}
	SECTION("Rz") {
		for (angle const& a : commmon_angles) {
			gate rz = gate_lib::rz(a);
			CHECK(rz.id() == gate_ids::rz);
			CHECK(rz.is(gate_ids::rz));
			CHECK(rz.axis() == rot_axis::z);
			CHECK(rz.is_one_qubit());
			CHECK_FALSE(rz.is_meta());
			CHECK_FALSE(rz.is_two_qubit());
			CHECK_FALSE(rz.is_r1());
			CHECK_FALSE(rz.is_measurement());
			CHECK(rz.rotation_angle() == a);
		}
	}
	SECTION("CRx") {
		for (angle const& a : commmon_angles) {
			gate crx = gate_lib::crx(a);
			CHECK(crx.id() == gate_ids::crx);
			CHECK(crx.is(gate_ids::crx));
			CHECK(crx.axis() == rot_axis::x);
			CHECK_FALSE(crx.is_one_qubit());
			CHECK_FALSE(crx.is_meta());
			CHECK(crx.is_two_qubit());
			CHECK_FALSE(crx.is_r1());
			CHECK_FALSE(crx.is_measurement());
			CHECK(crx.rotation_angle() == a);
		}
	}
	SECTION("CRy") {
		for (angle const& a : commmon_angles) {
			gate cry = gate_lib::cry(a);
			CHECK(cry.id() == gate_ids::cry);
			CHECK(cry.is(gate_ids::cry));
			CHECK(cry.axis() == rot_axis::y);
			CHECK_FALSE(cry.is_one_qubit());
			CHECK_FALSE(cry.is_meta());
			CHECK(cry.is_two_qubit());
			CHECK_FALSE(cry.is_r1());
			CHECK_FALSE(cry.is_measurement());
			CHECK(cry.rotation_angle() == a);
		}
	}
	SECTION("CRz") {
		for (angle const& a : commmon_angles) {
			gate crz = gate_lib::crz(a);
			CHECK(crz.id() == gate_ids::crz);
			CHECK(crz.is(gate_ids::crz));
			CHECK(crz.axis() == rot_axis::z);
			CHECK_FALSE(crz.is_one_qubit());
			CHECK_FALSE(crz.is_meta());
			CHECK(crz.is_two_qubit());
			CHECK_FALSE(crz.is_r1());
			CHECK_FALSE(crz.is_measurement());
			CHECK(crz.rotation_angle() == a);
		}
	}
	SECTION("NCRx") {
		for (angle const& a : commmon_angles) {
			gate ncrx = gate_lib::ncrx(a);
			CHECK(ncrx.id() == gate_ids::ncrx);
			CHECK(ncrx.is(gate_ids::ncrx));
			CHECK(ncrx.axis() == rot_axis::x);
			CHECK_FALSE(ncrx.is_one_qubit());
			CHECK_FALSE(ncrx.is_meta());
			CHECK_FALSE(ncrx.is_two_qubit());
			CHECK_FALSE(ncrx.is_r1());
			CHECK_FALSE(ncrx.is_measurement());
			CHECK(ncrx.rotation_angle() == a);
		}
	}
	SECTION("NCRy") {
		for (angle const& a : commmon_angles) {
			gate ncry = gate_lib::ncry(a);
			CHECK(ncry.id() == gate_ids::ncry);
			CHECK(ncry.is(gate_ids::ncry));
			CHECK(ncry.axis() == rot_axis::y);
			CHECK_FALSE(ncry.is_one_qubit());
			CHECK_FALSE(ncry.is_meta());
			CHECK_FALSE(ncry.is_two_qubit());
			CHECK_FALSE(ncry.is_r1());
			CHECK_FALSE(ncry.is_measurement());
			CHECK(ncry.rotation_angle() == a);
		}
	}
	SECTION("NCRz") {
		for (angle const& a : commmon_angles) {
			gate ncrz = gate_lib::ncrz(a);
			CHECK(ncrz.id() == gate_ids::ncrz);
			CHECK(ncrz.is(gate_ids::ncrz));
			CHECK(ncrz.axis() == rot_axis::z);
			CHECK_FALSE(ncrz.is_one_qubit());
			CHECK_FALSE(ncrz.is_meta());
			CHECK_FALSE(ncrz.is_two_qubit());
			CHECK_FALSE(ncrz.is_r1());
			CHECK_FALSE(ncrz.is_measurement());
			CHECK(ncrz.rotation_angle() == a);
		}
	}
}

TEST_CASE("Check gates adjointness", "[gate]")
{
	using namespace tweedledum;
	std::vector<gate> self_adjoint = {gate_lib::i,   gate_lib::h,   gate_lib::x,   gate_lib::y,
	                                  gate_lib::z,   gate_lib::cx,  gate_lib::cy,  gate_lib::cz,
	                                  gate_lib::ncx, gate_lib::ncy, gate_lib::ncz, gate_lib::swap};
	// nsa = not self-adjoint
	std::vector<gate> nsa = {gate_lib::s, gate_lib::t};
	std::vector<gate> nsa_dagger = {gate_lib::sdg, gate_lib::tdg};

	// add parameterisable gates
	std::vector<angle> commmon_angles = {sym_angle::zero, sym_angle::pi, sym_angle::pi_half,
	                                     sym_angle::pi_quarter};
	for (angle const& a : commmon_angles) {
		nsa.emplace_back(gate_lib::r1(a));
		nsa_dagger.emplace_back(gate_lib::r1(-a));
		nsa.emplace_back(gate_lib::rx(a));
		nsa_dagger.emplace_back(gate_lib::rx(-a));
		nsa.emplace_back(gate_lib::ry(a));
		nsa_dagger.emplace_back(gate_lib::ry(-a));
		nsa.emplace_back(gate_lib::rz(a));
		nsa_dagger.emplace_back(gate_lib::rz(-a));

		nsa.emplace_back(gate_lib::crx(a));
		nsa_dagger.emplace_back(gate_lib::crx(-a));
		nsa.emplace_back(gate_lib::cry(a));
		nsa_dagger.emplace_back(gate_lib::cry(-a));
		nsa.emplace_back(gate_lib::crz(a));
		nsa_dagger.emplace_back(gate_lib::crz(-a));

		nsa.emplace_back(gate_lib::ncrx(a));
		nsa_dagger.emplace_back(gate_lib::ncrx(-a));
		nsa.emplace_back(gate_lib::ncry(a));
		nsa_dagger.emplace_back(gate_lib::ncry(-a));
		nsa.emplace_back(gate_lib::ncrz(a));
		nsa_dagger.emplace_back(gate_lib::ncrz(-a));
	}
	REQUIRE(nsa.size() == nsa_dagger.size());

	for (uint32_t i = 0; i < self_adjoint.size(); ++i) {
		for (uint32_t j = 0; j < self_adjoint.size(); ++j) {
			if (i == j) {
				CHECK(self_adjoint[i].is_adjoint(self_adjoint[j]));
			} else {
				CHECK_FALSE(self_adjoint[i].is_adjoint(self_adjoint[j]));
			}
		}
		for (uint32_t j = 0; j < nsa.size(); ++j) {
			CHECK_FALSE(self_adjoint[i].is_adjoint(nsa[j]));
			CHECK_FALSE(nsa[j].is_adjoint(self_adjoint[i]));
			CHECK_FALSE(self_adjoint[i].is_adjoint(nsa_dagger[j]));
			CHECK_FALSE(nsa_dagger[j].is_adjoint(self_adjoint[i]));
		}
	}
	for (uint32_t i = 0; i < nsa.size(); ++i) {
		for (uint32_t j = 0; j < nsa_dagger.size(); ++j) {
			if (i == j) {
				CHECK(nsa[i].is_adjoint(nsa_dagger[j]));
				CHECK(nsa_dagger[j].is_adjoint(nsa[i]));
			} else {
				CHECK_FALSE(nsa[i].is_adjoint(nsa_dagger[j]));
				CHECK_FALSE(nsa_dagger[j].is_adjoint(nsa[i]));
			}
		}
	}
}