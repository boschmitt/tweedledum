/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/linear_synth.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/angle.hpp>
#include <tweedledum/utils/parity_terms.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Linear synthesis", "[linear_synth][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	parity_terms parities;
	parities.add_term(3u, angles::one_eighth);

	SECTION("Using binary strategy")
	{
		auto network = linear_synth<TestType>(3u, parities, {linear_synth_params::strategy::binary});
	}

	SECTION("Using gray strategy (default)")
	{
		auto network = linear_synth<TestType>(3u, parities);
	}
}
