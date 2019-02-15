/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/algorithms/synthesis/diagonal_synth.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Synthesize diagonal unitaries", "[diagonal_synth]")
{
	using namespace tweedledum;

	std::vector<double> angles = {1.0 / 2, 1.0 / 3, 1.0 / 4};
	const auto circ = diagonal_synth<netlist<mcst_gate>>(angles);
	std::stringstream str;
	write_quil(circ, str);
	CHECK(str.str() == R"quil(RZ(-0.208333) 0
RZ(-0.0416667) 1
CNOT 1 0
RZ(-0.291667) 0
CNOT 1 0
)quil");
}
