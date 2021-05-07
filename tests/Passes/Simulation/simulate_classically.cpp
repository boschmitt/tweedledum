/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Simulation/simulate_classically.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Reversible.h"
#include "tweedledum/Utils/DynamicBitset.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>

TEST_CASE("Simulate reversible circuit", "[simulate_classically]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();

    SECTION("Empty circuit")
    {
        DynamicBitset<uint8_t> pattern(circuit.num_qubits());
        do {
            auto result = simulate_classically(circuit, pattern);
            // Because of weird MSVC error I cannot do this:
            // CHECK(result == pattern);
            bool tmp = (result == pattern);
            CHECK(tmp);
            pattern.lexicographical_next();
        } while (!pattern.none());
    }
    SECTION("Inverting circuit")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q2});
        DynamicBitset<uint8_t> pattern(circuit.num_qubits());
        do {
            auto result = simulate_classically(circuit, pattern);
            // Because of weird MSVC error I cannot do this:
            // CHECK(~result == pattern);
            bool tmp = (~result == pattern);
            CHECK(tmp);
            pattern.lexicographical_next();
        } while (!pattern.none());
    }
    SECTION("Toffoli gate circuit")
    {
        std::vector<DynamicBitset<uint8_t>> permutation = {
          {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 7}, {3, 6}};
        circuit.apply_operator(Op::X(), {q1, q2, q0});
        for (uint32_t i = 0; i < permutation.size(); ++i) {
            DynamicBitset<uint8_t> pattern(3, i);
            auto result = simulate_classically(circuit, pattern);
            // Because of weird MSVC error I cannot do this:
            // CHECK(result == permutation[i]);
            bool tmp = (result == permutation[i]);
            CHECK(tmp);
        }
    }
}