/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <cstdint>
#include <tweedledum/algorithms/simulation/simulate_classically.hpp>
#include <tweedledum/algorithms/synthesis/tbs.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Transformation based synthesis", "[tbs][template]",
                           (gg_network, netlist), (mcmt_gate))
{
	std::vector<uint32_t> permutation = {0, 2, 3, 5, 7, 1, 4, 6};
	SECTION("Synthesize PRIME(3) - unidirectional TBS")
	{
		const auto network = tbs<TestType>(permutation);
		for (auto i = 0u; i < permutation.size(); ++i) {
			CHECK(simulate_classically(network, i) == permutation[i]);
		}
	}
	SECTION("Synthesize PRIME(3) - bidirectional TBS")
	{
		const auto network = tbs<TestType>(permutation);
		tbs_params params;
		params.behavior = tbs_params::behavior::bidirectional;
		for (auto i = 0u; i < permutation.size(); ++i) {
			CHECK(simulate_classically(network, i) == permutation[i]);
		}
	}
	SECTION("Synthesize PRIME(3) - multi-directional TBS")
	{
		const auto network = tbs<TestType>(permutation);
		tbs_params params;
		params.behavior = tbs_params::behavior::multidirectional;
		for (auto i = 0u; i < permutation.size(); ++i) {
			CHECK(simulate_classically(network, i) == permutation[i]);
		}
	}
}