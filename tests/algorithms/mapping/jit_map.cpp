/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/jit_map.hpp"

#include "test_circuits.hpp"
#include "tweedledum/algorithms/verification/map_verify.hpp"
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/mapped_dag.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/operations/w2_op.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Test for Just-in-time mapper", "[jit_map][mapping]", (op_dag),
                           (w2_op, w3_op, wn32_op))
{
	SECTION("Test circuit 00") {
		TestType original = test_circuit_00<TestType>();
		device device = device::path(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 01") {
		TestType original = test_circuit_01<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 02") {
		TestType original = test_circuit_02<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 03") {
		TestType original = test_circuit_03<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 04") {
		TestType original = test_circuit_04<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 05") {
		TestType original = test_circuit_05<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 06") {
		TestType original = test_circuit_06<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
	SECTION("Test circuit 07") {
		TestType original = test_circuit_07<TestType>();
		device device = device::ring(original.num_qubits());
		mapped_dag mapped = jit_map(original, device);
		CHECK(map_verify(original, mapped));
	}
}
