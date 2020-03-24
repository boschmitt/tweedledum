/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/dbs.hpp"

#include "tweedledum/algorithms/simulation/simulate_classically.hpp"
#include "tweedledum/algorithms/synthesis/stg.hpp"
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/operations/wn32_op.hpp"

#include <catch.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Decomposition based synthesis", "[dbs][template]",
                           (op_dag, netlist), (wn32_op))
{
	std::vector<uint32_t> permutation = {0, 2, 3, 5, 7, 1, 4, 6};
	SECTION("Synthesize PRIME(3) - PPRM")
	{
		const auto network = dbs<TestType>(permutation, stg_from_pprm());
		for (auto i = 0u; i < permutation.size(); ++i) {
			CHECK(simulate_classically(network, i) == permutation[i]);
		}
	}
	SECTION("Synthesize PRIME(3) - spectrum")
	{
		const auto network = dbs<TestType>(permutation, stg_from_spectrum());
		CHECK(network.num_operations() == 52u);
		CHECK(network.num_qubits() == 3u);
	}
}
