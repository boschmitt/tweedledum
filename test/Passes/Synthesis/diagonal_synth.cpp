/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Synthesis/diagonal_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Utils/Angle.h"

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
        WireRef q0 = expected.create_qubit();
        WireRef q1 = expected.create_qubit();
        WireRef q2 = expected.create_qubit();
        expected.apply_operator(Op::P(sym_angle::pi), {q1, q2, q0});

        std::vector<Angle> angles(7, sym_angle::zero);
        angles.push_back(sym_angle::pi);
        Circuit synthesized = diagonal_synth(angles, config);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Double-control Rx ~ CX") {
        Circuit expected;
        WireRef q0 = expected.create_qubit();
        WireRef q1 = expected.create_qubit();
        WireRef q2 = expected.create_qubit();
        expected.apply_operator(Op::Rx(sym_angle::pi), {q1, q2, q0});

        std::vector<Angle> angles(6, sym_angle::zero);
        angles.push_back(-sym_angle::pi_half);
        angles.push_back(sym_angle::pi_half);
        Circuit synthesized;
        synthesized.create_qubit();
        synthesized.create_qubit();
        synthesized.create_qubit();
        synthesized.apply_operator(Op::H(), {q0});
        diagonal_synth(synthesized, {q1, q2, q0}, angles, config);
        synthesized.apply_operator(Op::H(), {q0});
        CHECK(check_unitary(expected, synthesized));
    }
}