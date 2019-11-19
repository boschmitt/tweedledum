/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/cnot_rz.hpp>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/angle.hpp>
#include <tweedledum/utils/bit_matrix_rm.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("CNOT-RZ synthesis", "[cnot_rz][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	parity_terms terms;
	bit_matrix_rm<> transform(3u, 3u);
	transform.at(0, 0) = 1;
	transform.at(1, 1) = 1;
	transform.at(2, 2) = 1;
	SECTION("Trivial case") {
		terms.add_term(0b001, angles::pi_quarter);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 1u);
	}
	SECTION("Still trivial, but with more rotations") {
		terms.add_term(0b001, angles::pi_quarter);
		terms.add_term(0b010, angles::pi_quarter);
		terms.add_term(0b100, angles::pi_quarter);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 3u);
	}
	SECTION("Will require one CX") {
		transform.at(0, 1) = 1;
		terms.add_term(0b011, angles::pi_quarter);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 2u);
	}
	SECTION("Will require two CX") {
		transform.at(0, 1) = 1;
		transform.at(1, 2) = 1;
		terms.add_term(0b011, angles::pi_quarter);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 3u);
	}
	SECTION("Will require two CX") {
		transform.at(0, 1) = 1;
		transform.at(1, 2) = 1;
		terms.add_term(0b011, angles::pi_quarter);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 3u);
	}
	SECTION("Will require two CX") {
		terms.add_term(0b011, angles::pi_quarter);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 3u);
	}
	SECTION("Will require more CX") {
		constexpr auto T = angles::pi_quarter;
		constexpr auto T_dagger = -angles::pi_quarter;
		terms.add_term(0b001, T);
		terms.add_term(0b010, T);
		terms.add_term(0b100, T);
		terms.add_term(0b011, T_dagger);
		terms.add_term(0b101, T_dagger);
		terms.add_term(0b110, T_dagger);
		terms.add_term(0b111, T);
		auto network = cnot_rz<TestType>(transform, terms);
		CHECK(network.num_gates() == 13u);
	}
}
