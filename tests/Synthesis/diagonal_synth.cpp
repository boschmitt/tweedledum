/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/diagonal_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Utils/Numbers.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <nlohmann/json.hpp>
#include <vector>

TEST_CASE("Trivial cases for diagonal_synth", "[diagonal_synth][synth]")
{
    using namespace tweedledum;
    nlohmann::json config;
    SECTION("Double-control Z = CCZ")
    {
        std::vector<Qubit> qubits = {Qubit(0), Qubit(1), Qubit(2)};
        std::vector<double> angles(7, 0.0);
        angles.push_back(numbers::pi);
        do {
            Circuit expected;
            expected.create_qubit();
            expected.create_qubit();
            expected.create_qubit();
            expected.apply_operator(Op::P(numbers::pi), qubits);

            Circuit synthesized;
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.create_qubit();
            diagonal_synth(synthesized, qubits, {}, angles, config);
            CHECK(check_unitary(expected, synthesized));
        } while (std::next_permutation(qubits.begin(), qubits.end()));
    }
    SECTION("Double-control Rx ~ CCX")
    {
        std::vector<Qubit> qubits = {Qubit(0), Qubit(1), Qubit(2)};
        std::vector<double> angles(6, 0.0);
        angles.push_back(-numbers::pi_div_2);
        angles.push_back(numbers::pi_div_2);
        do {
            Circuit expected;
            expected.create_qubit();
            expected.create_qubit();
            expected.create_qubit();
            expected.apply_operator(Op::Rx(numbers::pi), qubits);

            Circuit synthesized;
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.apply_operator(Op::H(), {qubits.back()});
            diagonal_synth(synthesized, qubits, {}, angles, config);
            synthesized.apply_operator(Op::H(), {qubits.back()});
            CHECK(check_unitary(expected, synthesized));
        } while (std::next_permutation(qubits.begin(), qubits.end()));
    }
    SECTION("Double-control Rx (with first negative control) ~ CCX")
    {
        std::vector<Qubit> qubits = {Qubit(0), Qubit(1), Qubit(2)};
        std::vector<double> angles(6, 0.0);
        angles.push_back(-numbers::pi_div_2);
        angles.push_back(numbers::pi_div_2);
        do {
            std::vector<Qubit> qs = {!qubits[0], qubits[1], qubits[2]};
            Circuit expected;
            expected.create_qubit();
            expected.create_qubit();
            expected.create_qubit();
            expected.apply_operator(Op::Rx(numbers::pi), qs);

            Circuit synthesized;
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.apply_operator(Op::H(), {qubits.back()});
            diagonal_synth(synthesized, qs, {}, angles, config);
            synthesized.apply_operator(Op::H(), {qubits.back()});
            CHECK(check_unitary(expected, synthesized));
        } while (std::next_permutation(qubits.begin(), qubits.end()));
    }
    SECTION("Double-control Rx (with second negative control) ~ CCX")
    {
        std::vector<Qubit> qubits = {Qubit(0), Qubit(1), Qubit(2)};
        std::vector<double> angles(6, 0.0);
        angles.push_back(-numbers::pi_div_2);
        angles.push_back(numbers::pi_div_2);
        do {
            std::vector<Qubit> qs = {qubits[0], !qubits[1], qubits[2]};
            Circuit expected;
            expected.create_qubit();
            expected.create_qubit();
            expected.create_qubit();
            expected.apply_operator(Op::Rx(numbers::pi), qs);

            Circuit synthesized;
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.apply_operator(Op::H(), {qubits.back()});
            diagonal_synth(synthesized, qs, {}, angles, config);
            synthesized.apply_operator(Op::H(), {qubits.back()});
            CHECK(check_unitary(expected, synthesized));
        } while (std::next_permutation(qubits.begin(), qubits.end()));
    }
    SECTION("Double-control Rx (two negative controls) ~ CCX")
    {
        std::vector<Qubit> qubits = {Qubit(0), Qubit(1), Qubit(2)};
        std::vector<double> angles(6, 0.0);
        angles.push_back(-numbers::pi_div_2);
        angles.push_back(numbers::pi_div_2);
        do {
            std::vector<Qubit> qs = {!qubits[0], !qubits[1], qubits[2]};
            Circuit expected;
            expected.create_qubit();
            expected.create_qubit();
            expected.create_qubit();
            expected.apply_operator(Op::Rx(numbers::pi), qs);

            Circuit synthesized;
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.create_qubit();
            synthesized.apply_operator(Op::H(), {qubits.back()});
            diagonal_synth(synthesized, qs, {}, angles, config);
            synthesized.apply_operator(Op::H(), {qubits.back()});
            CHECK(check_unitary(expected, synthesized));
        } while (std::next_permutation(qubits.begin(), qubits.end()));
    }
}