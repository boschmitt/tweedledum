/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/linear_synth.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/utils/angle.hpp>
#include <tweedledum/utils/parity_terms.hpp>

TEST_CASE("Synthesize a simple function into a quantum network using linear_synth",
          "[linear_synth]")
{
	using namespace tweedledum;
	parity_terms parities;
	parities.add_term(3u, symbolic_angles::one_eighth);

	SECTION("Using binary strategy")
	{
		auto network = linear_synth<netlist<mcst_gate>>(3u, parities, {linear_synth_params::strategy::binary});
	}

	SECTION("Using gray strategy (default)")
	{
		auto network = linear_synth<netlist<mcst_gate>>(3u, parities);
	}
}
