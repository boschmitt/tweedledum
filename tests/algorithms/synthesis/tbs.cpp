/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/tbs.hpp"

#include "tweedledum/algorithms/simulation/simulate_classically.hpp"
#include "tweedledum/gates/wn32_op.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"

#include <catch.hpp>
#include <cstdint>
#include <vector>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Transformation based synthesis", "[tbs][template]",
                           (op_dag, netlist), (wn32_op))
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