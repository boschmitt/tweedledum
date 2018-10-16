/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tuple>
#include <vector>

TEST_CASE("Constructor", "[mcst_gate]")
{
	using namespace tweedledum;
	// Generate random qubit ids
	std::random_device rd;
	auto target_qid = rd();;
	auto control0_qid = rd();
	auto control1_qid = rd();
	INFO("Target qubit id is " << target_qid);
	INFO("Control qubits ids are " << control0_qid << " and "
	                               << control1_qid);

	SECTION("Single-qubit gate") {
		mcst_gate h_gate(gate_kinds_t::hadamard, target_qid);
		CHECK(h_gate.kind() == gate_kinds_t::hadamard);
		CHECK(h_gate.num_controls() == 0u);
		CHECK(h_gate.num_targets() == 1u);
		CHECK(h_gate.rotation_angle() == 0.0);
	}

	SECTION("Controlled gate") {
		auto control = std::vector<uint32_t>({control0_qid});
		auto target = std::vector<uint32_t>({target_qid});
		mcst_gate cx_gate(gate_kinds_t::cx, control, target);
		CHECK(cx_gate.kind() == gate_kinds_t::cx);
		CHECK(cx_gate.num_controls() == 1u);
		CHECK(cx_gate.num_targets() == 1u);
		CHECK(cx_gate.rotation_angle() == 0.0);

	}

	SECTION("Multiple controlled gate") {
		auto controls = std::vector<uint32_t>({control0_qid, control1_qid});
		auto target = std::vector<uint32_t>({target_qid});
		mcst_gate mcx_gate(gate_kinds_t::mcx, controls, target);
		CHECK(mcx_gate.kind() == gate_kinds_t::mcx);
		CHECK(mcx_gate.num_controls() == 2u);
		CHECK(mcx_gate.num_targets() == 1u);
		CHECK(mcx_gate.rotation_angle() == 0.0);
	}
}

TEST_CASE("Iterators", "[mcst_gate]")
{
	using namespace tweedledum;
	// Generate random qubit ids
	std::random_device rd;
	auto target_qid = rd();;
	auto control0_qid = rd();
	auto control1_qid = rd();
	if (control0_qid > control1_qid) {
		std::swap(control0_qid, control1_qid);
	}
	INFO("Target qubit id is " << target_qid);
	INFO("Control qubits ids are " << control0_qid << " and "
	                               << control1_qid);

	mcst_gate h_gate(gate_kinds_t::hadamard, target_qid);
	h_gate.foreach_target([&target_qid](auto qid) {
		CHECK(target_qid == qid);
	});
	h_gate.foreach_control([](auto qid) {
		// This function should not be called
		(void) qid;
		CHECK(0);
	});

	mcst_gate cx_gate(gate_kinds_t::cx,
	                  std::vector<uint32_t>({control0_qid}),
	                  std::vector<uint32_t>({target_qid}));
	cx_gate.foreach_target([&target_qid](auto qid) {
		CHECK(target_qid == qid);
	});
	cx_gate.foreach_control([&control0_qid](auto qid) {
		CHECK(control0_qid == qid);
	});

	mcst_gate mcx_gate(gate_kinds_t::mcx,
	                   std::vector<uint32_t>({control0_qid, control1_qid}),
	                   std::vector<uint32_t>({target_qid}));
	mcx_gate.foreach_target([&target_qid](auto qid) {
		CHECK(target_qid == qid);
	});
	auto i = 0u;
	mcx_gate.foreach_control([&i, &control0_qid, &control1_qid](auto qid) {
		CHECK(((i == 0 && control0_qid == qid) || (i == 1 && control1_qid == qid)) == true);
		++i;
	});
}

// namespace details {
// using namespace tweedledum;
// typedef std::tuple<mcst_gate, mcst_gate, bool> dep_table_t;

// // Dependency checks
// dep_table_t table[] = {
// #pragma region Two single-qubit gates acting on the same qubit
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::hadamard, 0), false},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::pauli_x, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::pauli_z, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::phase, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::phase_dagger, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::t, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::t_dagger, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::hadamard, 0), mcst_gate(gate_kinds_t::rotation_z, 0), true},

// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::pauli_x, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::pauli_z, 0), true},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::phase, 0), true},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::phase_dagger, 0), true},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::t, 0), true},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::t_dagger, 0), true},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::rotation_x, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_x, 0), mcst_gate(gate_kinds_t::rotation_z, 0), true},

// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::pauli_z, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::phase, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::pauli_z, 0), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::phase, 0), mcst_gate(gate_kinds_t::phase, 0), false},
// 	{mcst_gate(gate_kinds_t::phase, 0), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::phase, 0), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::phase, 0), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::phase, 0), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::phase, 0), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::phase_dagger, 0), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::phase_dagger, 0), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::phase_dagger, 0), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::phase_dagger, 0), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::phase_dagger, 0), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::t, 0), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::t, 0), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::t, 0), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::t, 0), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::t_dagger, 0), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::t_dagger, 0), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::t_dagger, 0), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::rotation_x, 0), mcst_gate(gate_kinds_t::rotation_x, 0), false},
// 	{mcst_gate(gate_kinds_t::rotation_x, 0), mcst_gate(gate_kinds_t::rotation_z, 0), true},

// 	{mcst_gate(gate_kinds_t::rotation_z, 0), mcst_gate(gate_kinds_t::rotation_z, 0), false},
// #pragma endregion
// #pragma region A single-qubit gate and a double-qubit gate
// 	// Single-qubit acting on the control
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::hadamard, 0), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_x, 0), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_z, 0), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase, 0), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::hadamard, 0), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_x, 0), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_z, 0), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase, 0), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	// Single-qubit acting on the target 
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::hadamard, 1), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_x, 1), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_z, 1), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase, 1), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase_dagger, 1), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t, 1), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t_dagger, 1), true},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_x, 1), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_z, 1), true},

// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::hadamard, 1), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_x, 1), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::pauli_z, 1), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase, 1), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::phase_dagger, 1), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t, 1), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::t_dagger, 1), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_x, 1), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::rotation_z, 1), false},
// #pragma endregion
// #pragma region A single-qubit gate and a multiple-qubit gate
// 	// Single-qubit acting on one of the controls
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::hadamard, 0), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_x, 0), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_z, 0), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase, 0), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::hadamard, 1), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_x, 1), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_z, 1), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase, 1), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase_dagger, 1), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t, 1), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t_dagger, 1), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_x, 1), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_z, 1), false},

// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::hadamard, 0), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_x, 0), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_z, 0), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase, 0), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t, 0), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t_dagger, 0), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_x, 0), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_z, 0), false},

// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::hadamard, 1), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_x, 1), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_z, 1), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase, 1), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase_dagger, 1), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t, 1), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t_dagger, 1), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_x, 1), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_z, 1), false},

// 	// Single-qubit acting on the target 
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::hadamard, 2), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_x, 2), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_z, 2), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase, 2), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase_dagger, 2), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t, 2), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t_dagger, 2), true},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_x, 2), false},
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_z, 2), true},

// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::hadamard, 2), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_x, 2), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::pauli_z, 2), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase, 2), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::phase_dagger, 2), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t, 2), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::t_dagger, 2), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_x, 2), true},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::rotation_z, 2), false},
// #pragma endregion
// #pragma region Two double-qubit gate
// 	// Same targets, same controls.
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), true},
// 	// Same targets, different controls
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({2u}), std::vector({1u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cz, std::vector({2u}), std::vector({1u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({2u}), std::vector({1u})), true},
// 	// Different targets, same controls
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({2u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({2u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({2u})), false},
// 	// Target of one acts on the control of the other
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({1u}), std::vector({2u})), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cz, std::vector({1u}), std::vector({2u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({1u}), std::vector({2u})), false},
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cz, std::vector({1u}), std::vector({2u})), true},
// 	// 
// 	{mcst_gate(gate_kinds_t::cx, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({1u}), std::vector({0u})), true},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cz, std::vector({1u}), std::vector({0u})), false},
// 	{mcst_gate(gate_kinds_t::cz, std::vector({0u}), std::vector({1u})), mcst_gate(gate_kinds_t::cx, std::vector({1u}), std::vector({0u})), true},
// #pragma endregion
// #pragma region A double-qubit gate and a multiple-qubit gate
// 	//!TODO
// #pragma endregion
// #pragma region Two multiple-qubit gate
// 	// Same targets, same controls.
// 	{mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), false},
// 	{mcst_gate(gate_kinds_t::mcz, std::vector({0u, 1u}), std::vector({2u})), mcst_gate(gate_kinds_t::mcx, std::vector({0u, 1u}), std::vector({2u})), true},
// 	//!TODO
// #pragma endregion
// };

// }; // namespace details

// TEST_CASE("Dependency", "[mcst_gate]")
// {
// 	using namespace tweedledum;
// 	auto table_size = (sizeof(details::table)/sizeof(*details::table));
// 	for (auto i = 0ull; i < table_size; ++i) {
// 		auto& gate0 = std::get<0>(details::table[i]);
// 		auto& gate1 = std::get<1>(details::table[i]);
// 		INFO("Gates:  " << gate0 << " and " << gate1);
// 		CHECK(gate0.is_dependent(gate1) == std::get<2>(details::table[i]));
// 		CHECK(gate1.is_dependent(gate0) == std::get<2>(details::table[i]));
// 	}
// }