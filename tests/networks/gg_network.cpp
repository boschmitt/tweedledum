/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include <catch.hpp>
#include <tweedledum/algorithms/remove_marked.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>

TEST_CASE("Remove marked nodes in gg_network", "gg_network")
{
	using namespace tweedledum;
	gg_network<mcst_gate> network;
	network.allocate_qubit();
	network.allocate_qubit();
	auto& a = network.add_gate(gate_kinds_t::hadamard, 0);
	auto& b = network.add_gate(gate_kinds_t::cx, 0, 1);
	auto& c = network.add_gate(gate_kinds_t::cx, 0, 1);
	auto& d = network.add_gate(gate_kinds_t::hadamard, 1);

	CHECK(2 == network.num_qubits());
	CHECK(4 == network.num_gates());

	network.mark(b, 1);
	network.mark(c, 1);

	CHECK(!network.mark(a));
	CHECK(network.mark(b));
	CHECK(network.mark(c));
	CHECK(!network.mark(d));
	network = remove_marked(network);

	CHECK(2 == network.num_qubits());
	CHECK(2 == network.num_gates());
}

TEST_CASE("Automark nodes in gg_network and remove them", "gg_network")
{
	using namespace tweedledum;
	gg_network<mcst_gate> network;
	network.allocate_qubit();
	network.allocate_qubit();
	auto& a = network.add_gate(gate_kinds_t::hadamard, 0);
	network.default_mark(1);
	auto& b = network.add_gate(gate_kinds_t::cx, 0, 1);
	auto& c = network.add_gate(gate_kinds_t::cx, 0, 1);
	network.default_mark(0);
	auto& d = network.add_gate(gate_kinds_t::hadamard, 1);

	CHECK(2 == network.num_qubits());
	CHECK(4 == network.num_gates());

	CHECK(!network.mark(a));
	CHECK(network.mark(b));
	CHECK(network.mark(c));
	CHECK(!network.mark(d));
	network = remove_marked(network);

	CHECK(2 == network.num_qubits());
	CHECK(2 == network.num_gates());
}
