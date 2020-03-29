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

TEST_CASE("Wire storage", "[wire_id]")
{
	using namespace tweedledum;
	
	wire::storage storage;
	SECTION("Using const literal strings") {
		wire_id qubit = storage.create_qubit("q0", wire_modes::inout);
		wire_id qubit_found = storage.wire("q0");
		CHECK(qubit == qubit_found);

		wire_id cbit = storage.create_cbit("c0", wire_modes::inout);
		wire_id cbit_found = storage.wire("c0");
		CHECK(cbit == cbit_found);
	}
	SECTION("Using strings") {
		std::string qubit_name = "__dum_q0";
		std::string cbit_name = "c0";

		wire_id qubit = storage.create_qubit(qubit_name, wire_modes::inout);
		wire_id cbit = storage.create_cbit(cbit_name, wire_modes::inout);
		wire_id qubit_found = storage.wire(qubit_name);
		wire_id cbit_found = storage.wire(cbit_name);
		CHECK(qubit == qubit_found);
		CHECK(cbit == cbit_found);
	}

}
