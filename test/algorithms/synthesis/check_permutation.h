/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/algorithms/simulation/simulate_classically.h"
#include "tweedledum/ir/Circuit.h"
#include "tweedledum/ir/Wire.h"
#include "tweedledum/support/DynamicBitset.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>
#include <vector>

using namespace tweedledum;

// TODO: use or remove!
// template<typename WordType>
// inline bool check_permutation(Circuit const& circuit,
//     std::vector<DynamicBitset<WordType>> const& permutation)
// {
// 	uint32_t const num_bits = permutation.at(0).size();
// 	assert(circuit.num_qubits() == num_bits);
// 	bool result = true;
// 	for (uint32_t i = 0; i < permutation.size() && result; ++i) {
// 		DynamicBitset<uint8_t> pattern(num_bits, i);
// 		auto sim_pattern = simulate_classically(circuit, pattern);
// 		result &= (sim_pattern == permutation[i]);
// 	}
// 	return result;
// }

inline bool check_permutation(Circuit const& circuit,
    std::vector<uint32_t> const& permutation)
{
	uint32_t const num_bits = circuit.num_qubits();
	assert(permutation.size() == (1u << num_bits));
	bool result = true;
	for (uint32_t i = 0; i < permutation.size() && result; ++i) {
		DynamicBitset<uint8_t> input(num_bits, i);
		DynamicBitset<uint8_t> expected(num_bits, permutation[i]);
		auto sim_pattern = simulate_classically(circuit, input);
		result &= (sim_pattern == expected);
	}
	return result;
}