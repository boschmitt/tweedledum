/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Utils/Angle.h"

#include <catch.hpp>

TEST_CASE("Create angles that don't need to be normalized", "[Angle]")
{
    using namespace tweedledum;

    Angle symbolic(1, 2);
    Angle numeric(0.5 * PI_k);

    SECTION("") {
        CHECK(!symbolic.is_numerically_defined());
        auto value = symbolic.symbolic_value();
        REQUIRE(value);
        auto [numerator, denominator] = value.value();
        CHECK(numerator == 1);
        CHECK(denominator == 2);
        REQUIRE(symbolic.numeric_value() == (0.5 * PI_k));
    }
    SECTION("An angle that can be simplied") {
        CHECK(numeric.is_numerically_defined());
        auto value = numeric.symbolic_value();
        REQUIRE(value.has_value() == false);
        REQUIRE(numeric.numeric_value() == (0.5 * PI_k));
    }
    CHECK(symbolic == numeric);
}

TEST_CASE("Normalizing angles", "[Angle]")
{
    using namespace tweedledum;

    Angle a(1, 2);
    Angle b(2, 4);
    Angle c(-2, -4);
    CHECK(a == b);
    CHECK(a == c);

    Angle d(-16, 32);
    Angle e(16, -32);
    CHECK(d == e);
    CHECK(a == -e);
    CHECK(-a == d);
}

TEST_CASE("Adding angles", "[Angle]")
{
    using namespace tweedledum;

    Angle a(1, 2);
    Angle b(1, 2);
    Angle c = a + b;
    auto value = c.symbolic_value();
    REQUIRE(value);
    auto [numerator, denominator] = value.value();
    CHECK(numerator == 1);
    CHECK(denominator == 1);
    CHECK(c.numeric_value() == PI_k);

    Angle d = c + c;
    CHECK(d == sym_angle::zero);
}