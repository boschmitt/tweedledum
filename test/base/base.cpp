/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/Standard.h"

#include "../check_unitary.h"

#include <cmath>
#define _USE_MATH_DEFINES
#include <catch.hpp>

TEST_CASE("Toffoli gate", "[base]")
{
    using namespace tweedledum;

    Circuit high_level;
    WireRef q0 = high_level.create_qubit();
    WireRef q1 = high_level.create_qubit();
    WireRef q2 = high_level.create_qubit();
    high_level.apply_operator(Op::X(), {q0, q1, q2});

    SECTION("Identified Phases") {
        Circuit decomposed;
        decomposed.create_qubit();
        decomposed.create_qubit();
        decomposed.create_qubit();
        decomposed.apply_operator(Op::H(), {q2});
        decomposed.apply_operator(Op::T(), {q0});
        decomposed.apply_operator(Op::T(), {q1});
        decomposed.apply_operator(Op::T(), {q2});
        decomposed.apply_operator(Op::X(), {q1, q2});
        decomposed.apply_operator(Op::Tdg(), {q2});
        decomposed.apply_operator(Op::X(), {q0, q2});
        decomposed.apply_operator(Op::T(), {q2});
        decomposed.apply_operator(Op::X(), {q1, q2});
        decomposed.apply_operator(Op::Tdg(), {q2});
        decomposed.apply_operator(Op::X(), {q0, q2});
        decomposed.apply_operator(Op::X(), {q0, q1});
        decomposed.apply_operator(Op::Tdg(), {q1});
        decomposed.apply_operator(Op::X(), {q0, q1});
        decomposed.apply_operator(Op::H(), {q2});
        CHECK(check_unitary(high_level, decomposed));
    }
    SECTION("Generic phase gates with symbolic angles") {
        Circuit decomposed;
        decomposed.create_qubit();
        decomposed.create_qubit();
        decomposed.create_qubit();
        decomposed.apply_operator(Op::H(), {q2});
        decomposed.apply_operator(Op::P(sym_angle::pi_quarter), {q0});
        decomposed.apply_operator(Op::P(sym_angle::pi_quarter), {q1});
        decomposed.apply_operator(Op::P(sym_angle::pi_quarter), {q2});
        decomposed.apply_operator(Op::X(), {q1, q2});
        decomposed.apply_operator(Op::P(-sym_angle::pi_quarter), {q2});
        decomposed.apply_operator(Op::X(), {q0, q2});
        decomposed.apply_operator(Op::P(sym_angle::pi_quarter), {q2});
        decomposed.apply_operator(Op::X(), {q1, q2});
        decomposed.apply_operator(Op::P(-sym_angle::pi_quarter), {q2});
        decomposed.apply_operator(Op::X(), {q0, q2});
        decomposed.apply_operator(Op::X(), {q0, q1});
        decomposed.apply_operator(Op::P(-sym_angle::pi_quarter), {q1});
        decomposed.apply_operator(Op::X(), {q0, q1});
        decomposed.apply_operator(Op::H(), {q2});
        CHECK(check_unitary(high_level, decomposed));
    }
    SECTION("Generic phase gates with numeric angles") {
        Circuit decomposed;
        decomposed.create_qubit();
        decomposed.create_qubit();
        decomposed.create_qubit();
        decomposed.apply_operator(Op::H(), {q2});
        decomposed.apply_operator(Op::P(M_PI_4), {q0});
        decomposed.apply_operator(Op::P(M_PI_4), {q1});
        decomposed.apply_operator(Op::P(M_PI_4), {q2});
        decomposed.apply_operator(Op::X(), {q1, q2});
        decomposed.apply_operator(Op::P(-M_PI_4), {q2});
        decomposed.apply_operator(Op::X(), {q0, q2});
        decomposed.apply_operator(Op::P(M_PI_4), {q2});
        decomposed.apply_operator(Op::X(), {q1, q2});
        decomposed.apply_operator(Op::P(-M_PI_4), {q2});
        decomposed.apply_operator(Op::X(), {q0, q2});
        decomposed.apply_operator(Op::X(), {q0, q1});
        decomposed.apply_operator(Op::P(-M_PI_4), {q1});
        decomposed.apply_operator(Op::X(), {q0, q1});
        decomposed.apply_operator(Op::H(), {q2});
        CHECK(check_unitary(high_level, decomposed));
    }
}
