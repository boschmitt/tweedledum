/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/algorithms/synthesis/diagonal_synth.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Synthesize diagonal unitaries", "[diagonal_synth][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	std::vector<double> angles = {1.0 / 2, 1.0 / 3, 1.0 / 4};
	const auto network = diagonal_synth<TestType>(angles);
	std::stringstream str;
	write_quil(network, str);
	CHECK(str.str() == R"quil(RZ(-0.20833333333333334) 0
RZ(-0.04166666666666663) 1
CNOT 1 0
RZ(-0.29166666666666663) 0
CNOT 1 0
)quil");
}
