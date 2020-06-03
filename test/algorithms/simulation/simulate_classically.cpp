/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/algorithms/simulation/simulate_classically.h"

#include "tweedledum/ir/Circuit.h"
#include "tweedledum/ir/GateLib.h"
#include "tweedledum/ir/Wire.h"
#include "tweedledum/support/DynamicBitset.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>

TEST_CASE("Simulate reversible circuit", "[simulate_classically]")
{
	using namespace tweedledum;
	Circuit circuit("my_circuit");
	WireRef q0 = circuit.create_qubit();
	WireRef q1 = circuit.create_qubit();
	WireRef q2 = circuit.create_qubit();

	SECTION("Empty circuit")
	{
		DynamicBitset<uint8_t> pattern(circuit.num_qubits());
		do {
			auto result = simulate_classically(circuit, pattern);
			CHECK(result == pattern);
			pattern.lexicographical_next();
		} while (!pattern.none());
	}
	SECTION("Inverting circuit")
	{
		circuit.create_instruction(GateLib::X(), {q0});
		circuit.create_instruction(GateLib::X(), {q1});
		circuit.create_instruction(GateLib::X(), {q2});
		DynamicBitset<uint8_t> pattern(circuit.num_qubits());
		do {
			auto result = simulate_classically(circuit, pattern);
			CHECK(~result == pattern);
			pattern.lexicographical_next();
		} while (!pattern.none());
	}
	SECTION("Toffoli gate circuit")
	{
		std::vector<DynamicBitset<uint8_t>> permutation = {{3, 0},
		    {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 7}, {3, 6}};
		circuit.create_instruction(GateLib::X(), {q1, q2, q0});
		for (uint32_t i = 0; i < permutation.size(); ++i) {
			DynamicBitset<uint8_t> pattern(3, i);
			auto result = simulate_classically(circuit, pattern);
			CHECK(result == permutation[i]);
		}
	}
}
