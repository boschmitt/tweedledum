/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <cmath>
#include <tweedledum/algorithms/synthesis/zyz.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;
TEST_CASE("ZYZ decompositions", "[zyz]")
{
	const auto hadamard = (1.0 / sqrt(2.0)) * single_qubit_unitary({{1, 1}, {1, -1}});

	netlist<io3_gate> circ;
	const auto q = circ.add_qubit();
	add_single_qubit_unitary(circ, q, hadamard);

	CHECK(circ.num_gates() == 1);
}
