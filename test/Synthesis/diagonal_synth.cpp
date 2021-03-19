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
    SECTION("Double-control Z") {
        Circuit expected;
        Qubit q0 = expected.create_qubit();
        Qubit q1 = expected.create_qubit();
        Qubit q2 = expected.create_qubit();
        expected.apply_operator(Op::P(numbers::pi), {q1, q2, q0});

        std::vector<double> angles(7, 0.0);
        angles.push_back(numbers::pi);
        Circuit synthesized = diagonal_synth(angles, config);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Double-control Rx ~ CX") {
        Circuit expected;
        Qubit q0 = expected.create_qubit();
        Qubit q1 = expected.create_qubit();
        Qubit q2 = expected.create_qubit();
        expected.apply_operator(Op::Rx(numbers::pi), {q1, q2, q0});

        std::vector<double> angles(6, 0.0);
        angles.push_back(-numbers::pi_div_2);
        angles.push_back(numbers::pi_div_2);
        Circuit synthesized;
        synthesized.create_qubit();
        synthesized.create_qubit();
        synthesized.create_qubit();
        synthesized.apply_operator(Op::H(), {q0});
        diagonal_synth(synthesized, {q1, q2, q0}, {}, angles, config);
        synthesized.apply_operator(Op::H(), {q0});
        CHECK(check_unitary(expected, synthesized));
    }
}