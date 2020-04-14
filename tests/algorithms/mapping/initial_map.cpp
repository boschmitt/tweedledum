/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/initial_map.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/mapped_dag.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Test for SAT intial map heuristic", "[initial_map][mapping]",
                           (netlist, op_dag), (w3_op, wn32_op))
{
	TestType network;
	initial_map_params params; 
	params.method = initial_map_params::methods::greedy_sat;

	SECTION("Empty network") {
		device device = device::path(network.num_qubits());
		std::vector<wire::id> map = initial_map(network, device);
		CHECK(map.size() == 0u);
	}
	SECTION("Simple circuit (UNSAT)") {
		wire::id q0 = network.create_qubit();
		network.create_cbit();
		wire::id q1 = network.create_qubit();
		network.create_cbit();
		wire::id q2 = network.create_qubit();
		network.create_cbit();

		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q2, q0);

		device device = device::path(network.num_qubits());
		std::vector<wire::id> map = initial_map(network, device, params);
		CHECK(map.size() == 3u);
	}
}

TEMPLATE_PRODUCT_TEST_CASE("Test for random intial map heuristic", "[initial_map][mapping]",
                           (netlist, op_dag), (w3_op, wn32_op))
{
	TestType network;
	initial_map_params params; 
	params.method = initial_map_params::methods::random;

	SECTION("Empty network") {
		device device = device::path(network.num_qubits());
		std::vector<wire::id> map = initial_map(network, device);
		CHECK(map.size() == 0u);
	}
	SECTION("Simple circuit (UNSAT)") {
		wire::id q0 = network.create_qubit();
		network.create_cbit();
		wire::id q1 = network.create_qubit();
		network.create_cbit();
		wire::id q2 = network.create_qubit();
		network.create_cbit();

		network.create_op(gate_lib::cx, q1, q0);
		network.create_op(gate_lib::cx, q1, q2);
		network.create_op(gate_lib::cx, q2, q0);

		device device = device::path(network.num_qubits());
		std::vector<wire::id> map = initial_map(network, device, params);
		CHECK(map.size() == 3u);
	}
}
