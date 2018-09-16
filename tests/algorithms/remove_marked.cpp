/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/remove_marked.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/networks/gates/gate_kinds.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/gdg.hpp>

TEST_CASE("Remove marked nodes", "[remove_marked]")
{
	using namespace tweedledum;
	gdg<qc_gate> network;

	CHECK(network.size() == 0);

	auto q0 = network.add_qubit("q0");
	network.add_gate(gate_kinds_t::hadamard, "q0");
	network.add_gate(gate_kinds_t::hadamard, "q0");
	gdg<qc_gate>::node_ptr_type node_ptr = {1, 0};
	auto& node = network.get_node(node_ptr);
	network.mark(node, 1);
	auto new_network = remove_marked(network);
	CHECK(new_network.size() == 3);
}
