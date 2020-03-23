/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/optimization/phase_folding.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/gates/w2_op.hpp"
#include "tweedledum/gates/w3_op.hpp"
#include "tweedledum/gates/wn32_op.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Phase folding", "[phase_folding][template]", (netlist, op_dag),
                           (w2_op, w3_op, wn32_op))
{
	TestType network;
	network.create_qubit("x1");
	network.create_qubit("x2");
	network.create_qubit("x3");
	network.create_qubit("x4");

	network.create_op(gate_lib::cx, "x3", "x4");

	network.create_op(gate_lib::t, "x1");
	network.create_op(gate_lib::t, "x4");

	network.create_op(gate_lib::cx, "x1", "x2");
	network.create_op(gate_lib::cx, "x3", "x4");

	network.create_op(gate_lib::cx, "x2", "x3");

	network.create_op(gate_lib::cx, "x2", "x1");
	network.create_op(gate_lib::cx, "x4", "x3");

	network.create_op(gate_lib::cx, "x2", "x3");

	network.create_op(gate_lib::cx, "x1", "x2");
	network.create_op(gate_lib::tdg, "x3");

	network.create_op(gate_lib::t, "x2");

	auto opt_network = phase_folding(network);
	// CHECK(check_optimized(network, opt_network));
}
