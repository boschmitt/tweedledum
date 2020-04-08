/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/placement/line_placement.hpp"

#include "../test_circuits.hpp"
#include "tweedledum/algorithms/verification/placement_verify.hpp"
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/mapped_dag.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>
#include <vector>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Test for line intial placement heuristic", "[line_placement][mapping]",
                           (netlist, op_dag), (w2_op, w3_op, wn32_op))
{
	TestType network;
	SECTION("Empty network") {
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 00") {
		network = test_circuit_00<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 01") {
		network = test_circuit_01<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 02") {
		network = test_circuit_02<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 03") {
		network = test_circuit_03<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 04") {
		network = test_circuit_04<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 05") {
		network = test_circuit_05<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 06") {
		network = test_circuit_06<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
	SECTION("Test circuit 07") {
		network = test_circuit_07<TestType>();
		device device = device::path(network.num_qubits());
		std::vector<wire::id> placement = detail::line_placement(network, device);
		CHECK(placement_verify(device, placement));
	}
}
