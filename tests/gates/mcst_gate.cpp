/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/gates/gate_base.hpp>
#include <tweedledum/utils/angle.hpp>
#include <vector>

TEST_CASE("MCST gate constructor", "[mcst_gate]")
{
	using namespace tweedledum;
	// Generate random qubit ids
	std::random_device rd;
	qubit_id target = rd();
	qubit_id control0 = rd();
	qubit_id control1 = rd();
	INFO("Target qubit id is " << target);
	INFO("Control qubits ids are " << control0 << " and " << control1);

	SECTION("Single-qubit gate")
	{
		mcst_gate h_gate(gate::hadamard, target);
		CHECK(h_gate.operation() == gate_set::hadamard);
		CHECK(h_gate.num_controls() == 0u);
		CHECK(h_gate.num_targets() == 1u);
		CHECK(h_gate.rotation_angle() == symbolic_angles::one_half);
	}

	SECTION("Controlled gate")
	{
		mcst_gate cx_gate(gate::cx, control0, target);
		CHECK(cx_gate.operation() == gate_set::cx);
		CHECK(cx_gate.num_controls() == 1u);
		CHECK(cx_gate.num_targets() == 1u);
		CHECK(cx_gate.rotation_angle() == symbolic_angles::one_half);

		// Using vectors to define a single controlled gate
		auto controls = std::vector<qubit_id>({control0});
		auto targets = std::vector<qubit_id>({target});
		mcst_gate cx_gate2(gate::cx, controls, targets);
		CHECK(cx_gate2.operation() == gate_set::cx);
		CHECK(cx_gate2.num_controls() == 1u);
		CHECK(cx_gate2.num_targets() == 1u);
		CHECK(cx_gate2.rotation_angle() == symbolic_angles::one_half);
	}

	SECTION("Multiple controlled gate")
	{
		auto controls = std::vector<qubit_id>({control0, control1});
		auto targets = std::vector<qubit_id>({target});
		mcst_gate mcx_gate(gate::mcx, controls, targets);
		CHECK(mcx_gate.operation() == gate_set::mcx);
		CHECK(mcx_gate.num_controls() == 2u);
		CHECK(mcx_gate.num_targets() == 1u);
		CHECK(mcx_gate.rotation_angle() == symbolic_angles::one_half);
	}
}

TEST_CASE("MCST gate iterators", "[mcst_gate]")
{
	using namespace tweedledum;
	// Generate random qubit ids
	std::random_device rd;
	qubit_id target = rd();
	qubit_id control0 = rd();
	qubit_id control1 = rd();
	if (control0 > control1) {
		std::swap(control0, control1);
	}
	INFO("Target qubit id is " << target);
	INFO("Control qubits ids are " << control0 << " and " << control1);

	SECTION("Single-qubit gate")
	{
		mcst_gate h_gate(gate::hadamard, target);
		h_gate.foreach_target([&target](auto qid) { CHECK(target == qid); });
		h_gate.foreach_control([](auto qid) {
			// This function should not be called
			(void) qid;
			CHECK(0);
		});
	}

	SECTION("Controlled gate")
	{
		auto control = std::vector<qubit_id>({control0});
		auto targets = std::vector<qubit_id>({target});
		mcst_gate cx_gate(gate::cx, control, targets);
		cx_gate.foreach_target([&target](auto qid) { CHECK(target == qid); });
		cx_gate.foreach_control([&control0](auto qid) { CHECK(control0 == qid); });
	}

	SECTION("Multiple controlled gate")
	{
		auto controls = std::vector<qubit_id>({control0, control1});
		auto targets = std::vector<qubit_id>({target});
		mcst_gate mcx_gate(gate::mcx, controls, targets);
		mcx_gate.foreach_target([&target](auto qid) { CHECK(target == qid); });
		auto i = 0u;
		mcx_gate.foreach_control([&i, &control0, &control1](auto qid) {
			CHECK(((i == 0 && control0 == qid) || (i == 1 && control1 == qid)) == true);
			++i;
		});
	}
}