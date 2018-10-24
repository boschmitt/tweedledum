/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/gray_synth.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>

TEST_CASE("Check example from original paper", "[gray_synth]")
{
	using namespace tweedledum;

	float T{0.39269908169872414}; // PI/8
	std::vector<std::pair<uint32_t, float>> parities{{0b0110, T}, {0b0001, T}, {0b1001, T}, {0b0111, T}, {0b1011, T}, {0b0011, T}};

	auto network = gray_synth<gg_network<mcst_gate>>(4, parities);
}
