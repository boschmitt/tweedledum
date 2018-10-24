/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <cstdint>
#include <tweedledum/algorithms/simulation/classical_simulation.hpp>
#include <tweedledum/algorithms/synthesis/transformation_based.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Verify result of TBS", "[classical_simulation]")
{
	using namespace tweedledum;
	std::vector<uint16_t> perm{{0, 2, 3, 5, 7, 1, 4, 6}};
	const auto perm_orig = perm; /* tbs changes perm */
	const auto net = transformation_based_synthesis<netlist<mcmt_gate>>(perm);

	for (auto i = 0u; i < perm.size(); ++i) {
		CHECK(simulate_pattern_classical(net, i) == perm_orig[i]);
	}
}
