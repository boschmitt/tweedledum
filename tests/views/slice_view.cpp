/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/networks/gates/gate_kinds.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/gdg.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/views/slice_view.hpp>
#include <tweedledum/views/depth_view.hpp>
#include <tweedledum/io/write_dot.hpp>

#include <iostream>

TEST_CASE("GDG Slice view", "[slice_view]")
{
	using namespace tweedledum;
	gdg<qc_gate> network;

	auto q0 = network.add_qubit("q0");
	auto q1 = network.add_qubit("q1");
	network.add_gate(gate_kinds_t::hadamard, "q0");
	network.add_gate(gate_kinds_t::hadamard, 0);
	network.add_gate(gate_kinds_t::t, 0);
	network.add_gate(gate_kinds_t::t_dagger, 0);
	network.add_gate(gate_kinds_t::rotation_z, "q0");

	slice_view slices(network);
	std::cout << "Number of slices: " << slices.depth() << "\n";

	depth_view levels(network);
	std::cout << "Number of levels: " << levels.depth() << "\n";
}

TEST_CASE("DAG PATH Slice view", "[slice_view]")
{
	using namespace tweedledum;
	dag_path<qc_gate> network;

	auto q0 = network.add_qubit("q0");
	auto q1 = network.add_qubit("q1");
	network.add_gate(gate_kinds_t::hadamard, "q0");
	network.add_gate(gate_kinds_t::hadamard, 0);
	network.add_gate(gate_kinds_t::t, 0);
	network.add_gate(gate_kinds_t::t_dagger, 0);
	network.add_gate(gate_kinds_t::rotation_z, "q0");

	slice_view slices(network);
	std::cout << "Number of slices: " << slices.depth() << "\n";

	depth_view levels(network);
	std::cout << "Number of levels: " << levels.depth() << "\n";

	write_dot(network, "path.dot");
}

