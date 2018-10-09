/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <cstdint>
#include <tweedledum/algorithms/synthesis/transformation_based.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

using namespace tweedledum;

TEST_CASE("Synthesize PRIME(3) with unidirectional transformation based synthesis",
          "[transformation_based]")
{
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	netlist<mcmt_gate> circ;
	transformation_based_synthesis(circ, permutation);

	for (auto i = 0; i < 8; ++i) {
		CHECK(i == permutation[i]);
	}
}

TEST_CASE("Synthesize PRIME(3) with bidirectional transformation based synthesis",
          "[transformation_based]")
{
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	netlist<mcmt_gate> circ;
	transformation_based_synthesis_bidirectional(circ, permutation);

	for (auto i = 0; i < 8; ++i) {
		CHECK(i == permutation[i]);
	}
}

TEST_CASE("Synthesize PRIME(3) with multi-directional transformation based synthesis",
          "[transformation_based]")
{
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	netlist<mcmt_gate> circ;
	transformation_based_synthesis_multidirectional(circ, permutation);

	for (auto i = 0; i < 8; ++i) {
		CHECK(i == permutation[i]);
	}
}
