/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/All.h"

#include "../check_unitary.h"

#include <cmath>
#define _USE_MATH_DEFINES
#include <catch.hpp>

TEST_CASE("Trivial cases for unitary builder", "[operators]")
{
    using namespace tweedledum;
    SECTION("Rxx") {
        Op::Rxx rxx(sym_angle::pi);
        Op::Unitary expected(rxx.matrix());

        Op::UnitaryBuilder builder(2);
        builder.apply_operator(rxx, std::vector({0u, 1u}));
        Op::Unitary built = builder.finished();

        CHECK(is_approx_equal(expected, built));
    }
    SECTION("Ryy") {
        Op::Ryy ryy(sym_angle::pi);
        Op::Unitary expected(ryy.matrix());

        Op::UnitaryBuilder builder(2);
        builder.apply_operator(Op::Ryy(sym_angle::pi), std::vector({0u, 1u}));
        Op::Unitary built = builder.finished();

        CHECK(is_approx_equal(expected, built));
    }
    SECTION("Rzz") {
        Op::Rzz rzz(sym_angle::pi);
        Op::Unitary expected(rzz.matrix());

        Op::UnitaryBuilder builder(2);
        builder.apply_operator(Op::Rzz(sym_angle::pi), std::vector({0u, 1u}));
        Op::Unitary built = builder.finished();

        CHECK(is_approx_equal(expected, built));
    }
}
