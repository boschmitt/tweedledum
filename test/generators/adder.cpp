/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/generators/adder.h"

#include "tweedledum/algorithms/simulation/simulate_classically.h"
#include "tweedledum/ir/Circuit.h"
#include "tweedledum/ir/GateLib.h"
#include "tweedledum/ir/Wire.h"
#include "tweedledum/support/DynamicBitset.h"

#include "tweedledum/export/to_qpic.h"

#include <catch.hpp>

TEST_CASE("Adder", "[adder][gen]")
{
	using namespace tweedledum;
	Circuit circuit("my_circuit");
	std::vector<WireRef> a;
	std::vector<WireRef> b;
	uint32_t n = 4;
	for (uint32_t i = 0; i < n; ++i) {
		a.push_back(circuit.create_qubit(fmt::format("a{}", i)));
	}
	for (uint32_t i = 0; i < n; ++i) {
		b.push_back(circuit.create_qubit(fmt::format("b{}", i)));
	}
	WireRef carry = circuit.create_qubit();
	carry_ripple_adder_inplace(circuit, a, b, carry);
	// to_qpic(std::cout, circuit);

	DynamicBitset<uint8_t> input(circuit.num_qubits(), (3 << n) + 4);
	DynamicBitset<uint8_t> expected(circuit.num_qubits(), (7 << n) + 4);
	auto sim_pattern = simulate_classically(circuit, input);
	CHECK(sim_pattern == expected);
}
