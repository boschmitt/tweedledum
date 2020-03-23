/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/gates/w3_op.hpp"
#include "tweedledum/gates/wn32_op.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Common functionality for all networks", "[networks][template]",
                           (netlist, op_dag), (w3_op, wn32_op))
{
	TestType network;
	SECTION("An empty network") {
		CHECK(network.size() == 0u);
		CHECK(network.num_wires() == 0u);
		CHECK(network.num_qubits() == 0u);
		CHECK(network.num_cbits() == 0u);
		CHECK(network.num_operations() == 0u);
	}
	SECTION("Reserving space") {
		uint32_t cap = network.capacity();
		network.reserve(cap << 2);
		CHECK(network.size() == 0u);
		CHECK(network.capacity() == (cap << 2));
		CHECK(network.num_wires() == 0u);
		CHECK(network.num_qubits() == 0u);
		CHECK(network.num_cbits() == 0u);
		CHECK(network.num_operations() == 0u);
	}
	SECTION("Creating wires") {
		for (uint32_t i = 0u; i < 8u; ++i) {
			std::string qname = fmt::format("q{}", i);
			std::string cname = fmt::format("c{}", i);
			wire_id nqubit = network.create_qubit(qname);
			wire_id qubit = network.create_qubit();
			wire_id ncbit = network.create_cbit(cname);
			wire_id cbit = network.create_cbit();

			CHECK(network.size() == ((i + 1) * 4));
			CHECK(network.num_wires() == ((i + 1) * 4));
			CHECK(network.num_qubits() == ((i + 1) * 2));
			CHECK(network.num_cbits() == ((i + 1) * 2));

			CHECK(network.wire_label(nqubit) == qname);
			CHECK(network.wire_label(qubit) == fmt::format("__q{}", (2 * i) + 1));
			CHECK(network.wire_label(!nqubit) == network.wire_label(nqubit));
			CHECK(network.wire_label(!qubit) == network.wire_label(qubit));

			CHECK(network.wire_label(ncbit) == cname);
			CHECK(network.wire_label(cbit) == fmt::format("__c{}", (2 * i) + 1));
			CHECK(network.wire_label(!ncbit) == network.wire_label(ncbit));
			CHECK(network.wire_label(!cbit) == network.wire_label(cbit));
		}
		CHECK(network.size() == 32u);
		CHECK(network.num_wires() == 32u);
		CHECK(network.num_qubits() == 16u);
		CHECK(network.num_cbits() == 16u);
		CHECK(network.num_operations() == 0u);
	}
	SECTION("One-qubit operations") {
		std::vector<gate> gates = {gate_lib::i, gate_lib::h,   gate_lib::x,
		                           gate_lib::y, gate_lib::z,   gate_lib::s,
		                           gate_lib::t, gate_lib::sdg, gate_lib::tdg};
		wire_id qubit = network.create_qubit();
		for (uint32_t i = 0; i < gates.size(); ++i) {
			node_id n_id = network.create_op(gates.at(i), qubit);
			auto node = network.node(n_id);
			CHECK(node.operation.gate.id() == gates.at(i).id());
			CHECK(node.operation.target() == qubit);
			CHECK(network.num_operations() == (i + 1));
		}
	}
	SECTION("Two-qubit operations") {
		std::vector<gate> gates = {gate_lib::cx, gate_lib::cy, gate_lib::cz, gate_lib::swap};
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		for (uint32_t i = 0; i < gates.size(); ++i) {
			node_id n_id = network.create_op(gates.at(i), q0, q1);
			auto node = network.node(n_id);
			CHECK(node.operation.gate.id() == gates.at(i).id());
			if (gates.at(i).id() == gate_ids::swap) {
				CHECK(node.operation.target(0) == q0);
				CHECK(node.operation.target(1) == q1);
			} else {
				CHECK(node.operation.control() == q0);
				CHECK(node.operation.target() == q1);
			}
			CHECK(network.num_operations() == (i + 1));
		}
	}
	SECTION("Three-qubit operations") {
		std::vector<gate> gates = {gate_lib::ncx, gate_lib::ncy, gate_lib::ncz};
		wire_id q0 = network.create_qubit();
		wire_id q1 = network.create_qubit();
		wire_id q2 = network.create_qubit();
		for (uint32_t i = 0; i < gates.size(); ++i) {
			node_id n_id = network.create_op(gates.at(i), q0, q1, q2);
			auto node = network.node(n_id);
			CHECK(node.operation.gate.id() == gates.at(i).id());
			CHECK(node.operation.control(0) == q0);
			CHECK(node.operation.control(1) == q1);
			CHECK(node.operation.target() == q2);
			CHECK(network.num_operations() == (i + 1));
		}
	}
}