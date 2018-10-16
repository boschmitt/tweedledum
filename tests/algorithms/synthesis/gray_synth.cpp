/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/gray_synth.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>

TEST_CASE("Check example from Amy paper", "gray_synth")
{
	using namespace tweedledum;
	gg_network<mcst_gate> network;
	std::vector<uint32_t> my_parities{0b011111, 0b100111, 0b100100, 0b001010};

	std::vector<uint32_t> p1{0b0110, 0b0001, 0b1001,
	                         0b0111, 0b1011, 0b0011};

	float T = 0.393;
	std::vector<float> Ts{T, T, T, T, T, T};

	gray_synth(network, 4, p1, Ts);

	//write_quil(network, std::cout);
}
