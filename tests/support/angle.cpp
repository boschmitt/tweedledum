/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/support/angle.hpp"

#include <catch.hpp>
#define _USE_MATH_DEFINES
#include <cmath>

TEST_CASE("Create angles that don't need to be normalized", "[angle]")
{
	using namespace tweedledum;

	angle symbolic(1, 2);
	angle numeric(0.5 * M_PI);

	SECTION("")
	{
		CHECK(!symbolic.is_numerically_defined());
		auto value = symbolic.symbolic_value();
		REQUIRE(value);
		auto [numerator, denominator] = value.value();
		CHECK(numerator == 1);
		CHECK(denominator == 2);
		REQUIRE(symbolic.numeric_value() == (0.5 * M_PI));
	}
	SECTION("An angle that can be simplied")
	{
		CHECK(numeric.is_numerically_defined());
		auto value = numeric.symbolic_value();
		REQUIRE(value.has_value() == false);
		REQUIRE(numeric.numeric_value() == (0.5 * M_PI));
	}
	CHECK(symbolic == numeric);
}

TEST_CASE("Normalizing angles", "[angle]")
{
	using namespace tweedledum;

	angle a(1, 2);
	angle b(2, 4);
	angle c(-2, -4);
	CHECK(a == b);
	CHECK(a == c);

	angle d(-16, 32);
	angle e(16, -32);
	CHECK(d == e);
	CHECK(a == -e);
	CHECK(-a == d);
}

TEST_CASE("Adding angles", "[angle]")
{
	using namespace tweedledum;

	angle a(1, 2);
	angle b(1, 2);
	angle c = a + b;
	auto value = c.symbolic_value();
	REQUIRE(value);
	auto [numerator, denominator] = value.value();
	CHECK(numerator == 1);
	CHECK(denominator == 1);
	CHECK(c.numeric_value() == M_PI);

	angle d = c + c;
	CHECK(d == sym_angle::zero);
}