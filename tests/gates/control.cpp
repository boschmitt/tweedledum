/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/control.hpp>

using namespace tweedledum;

TEST_CASE("Construct positive controls", "[control]")
{
	control_t c0(0);
	CHECK(c0.index() == 0);
	CHECK(!c0.is_complemented());

	control_t c1(1, false);
	CHECK(c1.index() == 1);
	CHECK(!c1.is_complemented());

	control_t c2 = 2;
	CHECK(c2.index() == 2);
	CHECK(!c2.is_complemented());
}

TEST_CASE("Construct negative controls", "[control]")
{
	control_t c0(0, true);
	CHECK(c0.index() == 0);
	CHECK(c0.is_complemented());

	control_t c1(1);
	c1.complement();
	CHECK(c1.index() == 1);
	CHECK(c1.is_complemented());

	control_t c2 = 2;
	c2 = !c2;
	CHECK(c2.index() == 2);
	CHECK(c2.is_complemented());
}

TEST_CASE("Compare controls", "[control]")
{
	control_t c1(0), c2(0, false), c3(0, true), c4(1);

	CHECK(c1 != c3);
	CHECK(c1 != c4);
	CHECK(c1 == c2);
	CHECK(c2 < c3);
	CHECK(c3 < c4);
}

TEST_CASE("Automatic conversion", "[control]")
{
  for (uint32_t i = 0; i < 10; ++i) {
    control_t c = i;
    CHECK(c.literal() == i << 1);
    CHECK(c.index() == i);
    CHECK(!c.is_complemented() );
    uint32_t j = c;
    CHECK(i == j);
  }

  for (uint32_t i = 0; i < 10; ++i) {
    control_t c(i, true);
    CHECK(c.literal() == (i << 1) + 1);
    CHECK(c.index() == i);
    CHECK(c.is_complemented() );
    uint32_t j = c;
    CHECK(i == j);
  }
}
