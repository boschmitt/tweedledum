/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/barenco_decomp.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Standard.h"

#include "../check_unitary.h"

#include <catch.hpp>

using namespace tweedledum;

inline bool check_decomp(Circuit const& left, Circuit const& right,
  double const rtol = 1e-05, double const atol = 1e-08)
{
    Op::UnitaryBuilder left_unitary(left.num_qubits());
    left.foreach_instruction([&](Instruction const& inst) {
        left_unitary.apply_operator(inst, inst.qubits());
    });
    Op::Unitary u_left = left_unitary.finished();

    Op::UnitaryBuilder right_unitary(right.num_qubits());
    right.foreach_instruction([&](Instruction const& inst) {
        right_unitary.apply_operator(inst, inst.qubits());
    });
    Op::Unitary u_right = right_unitary.finished();

    uint32_t const size = (1 << left.num_qubits());
    Op::Unitary t(u_right.matrix().block(0, 0, size, size));
    return is_approx_equal(u_left, t, rtol, atol);
}

// In this testcase the decomposition technique will automatically add a new
// clean ancilla to be able to realize the decomposition
TEST_CASE("Trivial clean ancilla barenco decomp", "[barenco_decomp][decomp]")
{
    using namespace tweedledum;
    nlohmann::json config;

    SECTION("(Mutiple) controlled X")
    {
        for (uint32_t i = 4u; i <= 9; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });
            original.apply_operator(Op::X(), qubits);

            Circuit decomposed = barenco_decomp(original, config);
            CHECK(check_decomp(original, decomposed));
        }
    }
    SECTION("(Mutiple) controlled Y")
    {
        for (uint32_t i = 1u; i <= 9; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });
            original.apply_operator(Op::Y(), qubits);

            Circuit decomposed = barenco_decomp(original, config);
            CHECK(check_decomp(original, decomposed));
        }
    }
    SECTION("(Mutiple) controlled Z")
    {
        for (uint32_t i = 1u; i <= 9; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });
            original.apply_operator(Op::Z(), qubits);

            Circuit decomposed = barenco_decomp(original, config);
            CHECK(check_decomp(original, decomposed));
        }
    }
}

// In this testcase we add an extra qubit to the circuit that the decomposition
// technique cannot know to be a clean ancilla, hance it treats it as dirty
TEST_CASE("Trivial dirty ancilla barenco decomp", "[barenco_decomp][decomp]")
{
    using namespace tweedledum;
    nlohmann::json config;

    SECTION("(Mutiple) controlled X")
    {
        for (uint32_t i = 4u; i <= 9; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });

            original.apply_operator(Op::X(), qubits);
            original.create_qubit(); // dirty ancilla

            Circuit decomposed = barenco_decomp(original, config);
            CHECK(original.num_qubits() == decomposed.num_qubits());
            CHECK(check_unitary(original, decomposed));
        }
    }
    SECTION("(Mutiple) controlled Y")
    {
        for (uint32_t i = 1u; i <= 9; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });

            original.apply_operator(Op::Y(), qubits);
            original.create_qubit(); // dirty ancilla

            Circuit decomposed = barenco_decomp(original, config);
            CHECK(original.num_qubits() == decomposed.num_qubits());
            CHECK(check_unitary(original, decomposed));
        }
    }
    SECTION("(Mutiple) controlled Z")
    {
        for (uint32_t i = 1u; i <= 9; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });

            original.apply_operator(Op::Z(), qubits);
            original.create_qubit(); // dirty ancilla

            Circuit decomposed = barenco_decomp(original, config);
            CHECK(original.num_qubits() == decomposed.num_qubits());
            CHECK(check_unitary(original, decomposed));
        }
    }
}

TEST_CASE("Trivial clean V ancilla barenco decomp", "[barenco_decomp][decomp]")
{
    using namespace tweedledum;
    nlohmann::json config;

    SECTION("(Mutiple) controlled X")
    {
        for (uint32_t i = 4u; i <= 7; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });

            original.apply_operator(Op::X(), qubits);
            config["max_qubits"] = i + (i - 2 - 1);
            Circuit decomposed = barenco_decomp(original, config);
            CHECK(check_decomp(original, decomposed));
        }
    }
    SECTION("(Mutiple) controlled Y")
    {
        for (uint32_t i = 4u; i <= 7; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });

            original.apply_operator(Op::Y(), qubits);
            config["max_qubits"] = i + (i - 2 - 1);
            Circuit decomposed = barenco_decomp(original, config);
            CHECK(check_decomp(original, decomposed));
        }
    }
    SECTION("(Mutiple) controlled Z")
    {
        for (uint32_t i = 4u; i <= 7; ++i) {
            Circuit original;
            std::vector<Qubit> qubits(i, Qubit::invalid());
            std::generate(qubits.begin(), qubits.end(),
              [&]() { return original.create_qubit(); });

            original.apply_operator(Op::Z(), qubits);
            config["max_qubits"] = i + (i - 2 - 1);
            Circuit decomposed = barenco_decomp(original, config);
            CHECK(check_decomp(original, decomposed));
        }
    }
}
