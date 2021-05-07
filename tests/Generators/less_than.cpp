/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Generators/less_than.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Passes/Simulation/simulate_classically.h"
#include "tweedledum/Utils/DynamicBitset.h"

#include <catch.hpp>

bool validate_lt(tweedledum::Circuit const& circuit, uint32_t n)
{
    using namespace tweedledum;
    bool is_valid = true;
    uint32_t const n_qubits = circuit.num_qubits();
    for (uint32_t a = 0; a < (1u << n); ++a) {
        for (uint32_t b = 0; b < (1u << n); ++b) {
            DynamicBitset<uint8_t> input(n_qubits, (b << n) + a);
            DynamicBitset<uint8_t> expected(n_qubits, (b << n) + a);
            expected[2 * n] = (a < b);
            auto sim_pattern = simulate_classically(circuit, input);
            is_valid &= (sim_pattern == expected);
        }
    }
    return is_valid;
}

TEST_CASE("Less than (<)", "[lt][generators]")
{
    using namespace tweedledum;
    uint32_t n = 5;
    Circuit circuit = less_than(n);
    CHECK(validate_lt(circuit, n));
}
