/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/diagonal_synth.h"

#include "tweedledum/algorithms/verification/unitary_verify.h"
#include "tweedledum/ir/Circuit.h"
#include "tweedledum/ir/GateLib.h"
#include "tweedledum/ir/Wire.h"

#include <catch.hpp>

TEST_CASE("Synthesize diagonal unitaries", "[diagonal][synth]")
{
	using namespace tweedledum;
	SECTION("Three controls Z")
	{
		std::vector<double> angles(7, 0.);
		angles.push_back(M_PI);
		Circuit circuit0 = diagonal_synth(angles);

		Circuit circuit1("expected");
		WireRef q0 = circuit1.create_qubit();
		WireRef q1 = circuit1.create_qubit();
		WireRef q2 = circuit1.create_qubit();
		circuit1.create_instruction(GateLib::R1(M_PI), {q1, q2, q0});
		CHECK(unitary_verify(circuit0, circuit1));
	}
}
