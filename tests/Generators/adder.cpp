/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Generators/adder.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Passes/Simulation/simulate_classically.h"
#include "tweedledum/Utils/DynamicBitset.h"

#include <catch.hpp>

bool validate_adder(tweedledum::Circuit const& circuit, uint32_t n)
{
    using namespace tweedledum;
    bool is_valid = true;
    uint32_t const n_qubits = circuit.num_qubits();
    for (uint32_t a = 0; a < (1u << n); ++a) {
        for (uint32_t b = 0; b < (1u << n); ++b) {
            uint32_t sum = a + b;
            DynamicBitset<uint8_t> input(n_qubits, (b << n) + a);
            DynamicBitset<uint8_t> expected(n_qubits, (sum << n) + a);
            auto sim_pattern = simulate_classically(circuit, input);
            is_valid &= (sim_pattern == expected);
        }
    }
    return is_valid;
}

TEST_CASE("Adder", "[adder][generators]")
{
    using namespace tweedledum;
    Circuit circuit;
    std::vector<Qubit> a_qubits;
    std::vector<Qubit> b_qubits;
    uint32_t n = 4;
    for (uint32_t i = 0; i < n; ++i) {
        a_qubits.push_back(circuit.create_qubit(fmt::format("a{}", i)));
    }
    for (uint32_t i = 0; i < n; ++i) {
        b_qubits.push_back(circuit.create_qubit(fmt::format("b{}", i)));
    }
    Qubit carry = circuit.create_qubit();
    carry_ripple_adder_inplace(circuit, a_qubits, b_qubits, carry);
    CHECK(validate_adder(circuit, n));
}

