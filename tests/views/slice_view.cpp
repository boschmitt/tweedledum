/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gdg_network.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/views/slice_view.hpp>
#include <tweedledum/views/depth_view.hpp>
#include <tweedledum/io/write_dot.hpp>

#include <iostream>

TEST_CASE("GDG Slice view", "[slice_view]")
{
	using namespace tweedledum;
	gdg_network<mcst_gate> network;

	auto q0 = network.add_qubit("q0");
	auto q1 = network.add_qubit("q1");
	network.add_gate(gate_kinds_t::hadamard, "q0");
	network.add_gate(gate_kinds_t::hadamard, 0);
	network.add_gate(gate_kinds_t::t, 0);
	network.add_gate(gate_kinds_t::cx, "q1", "q0");
	network.add_gate(gate_kinds_t::cx, "q1", "q0");
	network.add_gate(gate_kinds_t::t_dagger, 0);
	network.add_gate(gate_kinds_t::rotation_z, "q0");

	slice_view slices(network);
	std::cout << "Number of slices: " << slices.num_slices() << "\n";

	depth_view levels(network);
	std::cout << "Number of levels: " << levels.depth() << "\n";
	write_dot(network, "gdg_network.dot");
}

TEST_CASE("DAG PATH Slice view", "[slice_view]")
{
	using namespace tweedledum;
	gg_network<mcst_gate> network;

	auto q0 = network.add_qubit("q0");
	auto q1 = network.add_qubit("q1");
	network.add_gate(gate_kinds_t::hadamard, "q0");
	network.add_gate(gate_kinds_t::hadamard, 0);
	network.add_gate(gate_kinds_t::t, 0);
	network.add_gate(gate_kinds_t::cx, "q1", "q0");
	network.add_gate(gate_kinds_t::cx, "q1", "q0");
	network.add_gate(gate_kinds_t::t_dagger, 0);
	network.add_gate(gate_kinds_t::rotation_z, "q0");

	slice_view slices(network);
	std::cout << "Number of slices: " << slices.num_slices() << "\n";

	depth_view levels(network);
	std::cout << "Number of levels: " << levels.depth() << "\n";

	write_dot(network, "path.dot");
}

