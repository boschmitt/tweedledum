/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Passes/Simulation/simulate_classically.h"
#include "tweedledum/Utils/DynamicBitset.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>
#include <vector>

using namespace tweedledum;

inline bool check_permutation(
  Circuit const& circuit, std::vector<uint32_t> const& permutation)
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