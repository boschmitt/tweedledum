/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/networks/mapped_dag.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/gates/w3_op.hpp"
#include "tweedledum/gates/wn32_op.hpp"
#include "tweedledum/io/write_utf8.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Basic mapped network functionality", "[mapped_dag][template]",
                           (netlist, op_dag), (w3_op, wn32_op))
{
	TestType network;
	SECTION("An empty network") {
		device arch = device::path(network.num_qubits());
		mapped_dag mapped(network, arch);
		CHECK(mapped.size() == 0u);
		CHECK(mapped.num_wires() == 0u);
		CHECK(mapped.num_qubits() == 0u);
		CHECK(mapped.num_cbits() == 0u);
		CHECK(mapped.num_operations() == 0u);
	}
	SECTION("Axxnetwork") {
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		network.create_op(gate_lib::h, q0);
		network.create_op(gate_lib::cx, q2, q0);

		device arch = device::path(network.num_qubits());
		mapped_dag mapped(network, arch);
		CHECK((mapped.create_op(gate_lib::h, q0) != node::invalid));
		CHECK((mapped.create_op(gate_lib::cx, q2, q0) == node::invalid));
		mapped.create_swap(q0, q1);
		CHECK((mapped.create_op(gate_lib::cx, q2, q0) != node::invalid));
	}
}