/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/ir/Unitary.h"

#include "tweedledum/ir/GateLib.h"
#include "tweedledum/ir/Wire.h"

#include <catch.hpp>

TEST_CASE("Basic unitary functionality", "[unitary][ir]")
{
	using namespace tweedledum;
	SECTION("An empty unitary")
	{
		Unitary u("unitary");
		CHECK(u.num_wires() == 0u);
		CHECK(u.num_qubits() == 0u);
	}
	SECTION("One-qubit unitary, no instruction unitary")
	{
		Unitary u("unitary");
		u.create_qubit();
		CHECK(u.num_wires() == 1u);
		CHECK(u.num_qubits() == 1u);
	}
	SECTION("One-qubit unitary, one instruction unitary")
	{
		Unitary u("unitary");
		WireRef q0 = u.create_qubit();
		u.create_instruction(GateLib::X(), {q0});
		CHECK(u.num_wires() == 1u);
		CHECK(u.num_qubits() == 1u);
	}
}
