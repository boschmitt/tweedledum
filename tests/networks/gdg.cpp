/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/networks/gates/gate_kinds.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/gdg.hpp>

TEST_CASE("Create GDG network with a few qubits", "[gdg]")
{
	using namespace tweedledum;
	gdg<qc_gate> network;

	CHECK(network.size() == 0);
	
	auto q0 = network.add_qubit("q0");
	CHECK(network.size() == 2);
	CHECK(network.num_qubits() == 1);

	auto q1 = network.add_qubit();
	CHECK(network.size() == 4);
	CHECK(network.num_qubits() == 2);
}
