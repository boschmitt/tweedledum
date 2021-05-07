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

inline bool check_prime(Circuit const& circuit)
{
    uint32_t const num_vars = circuit.num_qubits() - 1u;
    kitty::dynamic_truth_table model(num_vars);
    kitty::create_prime(model);

    uint32_t const target = circuit.num_qubits() - 1u;
    uint32_t const num_patterns = model.num_bits();
    DynamicBitset<uint8_t> pattern(circuit.num_qubits());
    kitty::dynamic_truth_table result_tt(num_vars);
    for (uint32_t i = 0u; i < num_patterns; ++i) {
        auto result = simulate_classically(circuit, pattern);
        if (result[target]) {
            kitty::set_bit(result_tt, i);
        }
        pattern.lexicographical_next();
    }
    return result_tt == model;
}