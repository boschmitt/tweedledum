/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/transformations/asap_reschedule.hpp"

#include "tweedledum/algorithms/analysis/check_layerized.hpp"
#include "tweedledum/algorithms/verification/unitary_verify.hpp"
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/operations/w2_op.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/views/layers_view.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("ASAP reschedule", "[asap_reschedule][transformations]", (op_dag),
                           (w2_op, w3_op, wn32_op))
{
	TestType network;
	wire_id const q0 = network.create_qubit();
	wire_id const q1 = network.create_qubit();
	wire_id const q2 = network.create_qubit();

	network.create_op(gate_lib::h, q0);
	network.create_op(gate_lib::cz, q1, q0);
	network.create_op(gate_lib::h, q0);
	network.create_op(gate_lib::h, q2);
	CHECK_FALSE(check_layerized(network));

	TestType rescheduled = asap_reschedule(network);
	CHECK(rescheduled.node(node_id(4u)).op.is(gate_ids::h));
	CHECK(check_layerized(rescheduled));
	CHECK(unitary_verify(network, rescheduled));
}