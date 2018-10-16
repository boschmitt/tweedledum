/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Giulia Meuli
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "gray_synth.hpp"
#include "lin_comb_synth.hpp"

#include <cstdint>
#include <iostream>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <vector>

namespace tweedledum {

enum class pair_type {prop1, prop2};

struct optimized_esop
{
	public: 
	std::vector<kitty::cube> opt_esop;
	std::vector<pair_type> pairing;

	optimized_esop(std::vector<kitty::cube> esop)
	{
		auto g = optimization_graph(esop);

	}
};


/* gets a vect of cubes with the first num_pairs that can be paired according to the pairing type
generate the resulting network of cubes */
template<class Network>
void opt_stg_from_esop (Network& net, optimized_esop pairing, std::vector<uint8_t> const& qubit_map)
{
	/*foreach elem into pairing, put the first two cubes into a function that gets the eq circuit and add the gate to the network/*
	*/

}

} /* namespace tweedledum */
