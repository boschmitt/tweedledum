/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/linear_synth.hpp"

#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/support/angle.hpp"
#include "tweedledum/support/parity_terms.hpp"

#include <catch.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Linear synthesis", "[linear_synth][template]",
                           (op_dag, netlist), (wn32_op, w3_op))
{
	parity_terms<uint32_t> parities;
	parities.add_term(3u, sym_angle::pi_quarter);

	SECTION("Using binary strategy")
	{
		auto network = linear_synth<TestType>(3u, parities, {linear_synth_params::strategy::binary});
	}

	SECTION("Using gray strategy (default)")
	{
		auto network = linear_synth<TestType>(3u, parities);
	}
}
