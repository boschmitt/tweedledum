/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/networks/wire_id.hpp"

#include <catch.hpp>

TEST_CASE("Wire id constructors", "[wire_id]")
{
	using namespace tweedledum;
	SECTION("Simple qubit id") {
		wire_id id(0, true);
		CHECK(id.is_qubit());
		CHECK_FALSE(id.is_complemented());
	}
	SECTION("Simple classical bit id") {
		wire_id id(0, false);
		CHECK_FALSE(id.is_qubit());
		CHECK_FALSE(id.is_complemented());
	}
	SECTION("Complementing ids") {
		wire_id id(10, true);
		wire_id cmpl_id = !id;
		CHECK_FALSE(id == cmpl_id);
		CHECK(id == !cmpl_id);
		CHECK(!id == cmpl_id);
		CHECK(cmpl_id.id() == 10u);
		CHECK(cmpl_id.is_qubit());
		CHECK(cmpl_id.is_complemented());
	}
}
