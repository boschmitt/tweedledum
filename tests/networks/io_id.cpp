/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/networks/io_id.hpp>

TEST_CASE("I/O id constructors", "[io_id]")
{
	using namespace tweedledum;
	SECTION("Simple qubit id") {
		io_id id(0, true);
		CHECK(id.literal() == 0);
		CHECK(id.is_qubit());
		CHECK_FALSE(id.is_complemented());
	}
	SECTION("Simple cbit id") {
		io_id id(0, false);
		CHECK(id.literal() == 0);
		CHECK_FALSE(id.is_qubit());
		CHECK_FALSE(id.is_complemented());
	}
	SECTION("Complementing qubit ids") {
		io_id id(10, true);
		io_id cmpl_id = !id;
		CHECK(id.literal() == 20);
		CHECK(cmpl_id.literal() == 21);
		CHECK(cmpl_id.index() == 10);
		CHECK(cmpl_id.is_qubit());
		CHECK(cmpl_id.is_complemented());
	}
}
