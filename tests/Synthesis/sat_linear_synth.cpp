/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/sat_linear_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Target/Device.h"
#include "tweedledum/Utils/Matrix.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Trivial cases for non-constrained SAT linear synthesis",
  "[sat_linear_synth][synth]")
{
    using namespace tweedledum;
    BMatrix transform = BMatrix::Identity(3, 3);
    Circuit expected;
    Qubit q0 = expected.create_qubit();
    Qubit q1 = expected.create_qubit();
    Qubit q2 = expected.create_qubit();
    // clang-format off
    SECTION("Identity")
    {
        transform << 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Swap (q0 , q1)")
    {
        expected.apply_operator(Op::Swap(), {q0, q1});
        transform << 0, 1, 0,
                     1, 0, 0,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Swap (q0 , q2)")
    {
        expected.apply_operator(Op::Swap(), {q0, q2});
        transform << 0, 0, 1,
                     0, 1, 0,
                     1, 0, 0;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("2 Swaps (q1, q2) (q0, 1)")
    {
        expected.apply_operator(Op::Swap(), {q1, q2});
        expected.apply_operator(Op::Swap(), {q0, q1});
        transform << 0, 0, 1, 
                     1, 0, 0,
                     0, 1, 0;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Upper triangle")
    {
        expected.apply_operator(Op::X(), {q2, q1});
        expected.apply_operator(Op::X(), {q1, q0});
        transform << 1, 1, 1,
                     0, 1, 1,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Lower triangle")
    {
        expected.apply_operator(Op::X(), {q0, q1});
        expected.apply_operator(Op::X(), {q1, q2});
        transform << 1, 0, 0,
                     1, 1, 0,
                     1, 1, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Two CX to q0")
    {
        expected.apply_operator(Op::X(), {q1, q0});
        expected.apply_operator(Op::X(), {q2, q0});
        transform << 1, 1, 1,
                     0, 1, 0,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Two CX to q1")
    {
        expected.apply_operator(Op::X(), {q0, q1});
        expected.apply_operator(Op::X(), {q2, q1});
        transform << 1, 0, 0,
                     1, 1, 1,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Two CX to q2")
    {
        expected.apply_operator(Op::X(), {q0, q2});
        expected.apply_operator(Op::X(), {q1, q2});
        transform << 1, 0, 0,
                     0, 1, 0,
                     1, 1, 1;
        Circuit synthesized = sat_linear_synth(transform);
        CHECK(check_unitary(expected, synthesized));
    }
    // clang-format on
}

TEST_CASE("Trivial path cases for constrained SAT linear synthesis",
  "[sat_linear_synth][synth]")
{
    using namespace tweedledum;
    Device path_3 = Device::path(3);
    BMatrix transform = BMatrix::Identity(3, 3);
    Circuit expected;
    Qubit q0 = expected.create_qubit();
    Qubit q1 = expected.create_qubit();
    Qubit q2 = expected.create_qubit();
    // clang-format off
    SECTION("Identity")
    {
        transform << 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Swap (q0 , q1)")
    {
        expected.apply_operator(Op::Swap(), {q0, q1});
        transform << 0, 1, 0,
                     1, 0, 0,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Swap (q0 , q2)")
    {
        expected.apply_operator(Op::Swap(), {q0, q2});
        transform << 0, 0, 1,
                     0, 1, 0,
                     1, 0, 0;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("2 Swaps (q1, q2) (q0, 1)")
    {
        expected.apply_operator(Op::Swap(), {q1, q2});
        expected.apply_operator(Op::Swap(), {q0, q1});
        transform << 0, 0, 1, 
                     1, 0, 0,
                     0, 1, 0;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Upper triangle")
    {
        expected.apply_operator(Op::X(), {q2, q1});
        expected.apply_operator(Op::X(), {q1, q0});
        transform << 1, 1, 1,
                     0, 1, 1,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Lower triangle")
    {
        expected.apply_operator(Op::X(), {q0, q1});
        expected.apply_operator(Op::X(), {q1, q2});
        transform << 1, 0, 0,
                     1, 1, 0,
                     1, 1, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Two CX to q0")
    {
        expected.apply_operator(Op::X(), {q1, q0});
        expected.apply_operator(Op::X(), {q2, q0});
        transform << 1, 1, 1,
                     0, 1, 0,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Two CX to q1")
    {
        expected.apply_operator(Op::X(), {q0, q1});
        expected.apply_operator(Op::X(), {q2, q1});
        transform << 1, 0, 0,
                     1, 1, 1,
                     0, 0, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    SECTION("Two CX to q2")
    {
        expected.apply_operator(Op::X(), {q0, q2});
        expected.apply_operator(Op::X(), {q1, q2});
        transform << 1, 0, 0,
                     0, 1, 0,
                     1, 1, 1;
        Circuit synthesized = sat_linear_synth(path_3, transform);
        CHECK(check_unitary(expected, synthesized));
    }
    // clang-format on
}