/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/generators/less_than.h"

#include "tweedledum/algorithms/simulation/simulate_classically.h"
#include "tweedledum/ir/Circuit.h"
#include "tweedledum/ir/GateLib.h"
#include "tweedledum/ir/Wire.h"
#include "tweedledum/support/DynamicBitset.h"

#include "tweedledum/export/to_qpic.h"

#include <catch.hpp>

bool validate_lt(tweedledum::Circuit const& circuit, uint32_t n)
{
	using namespace tweedledum;
	bool is_valid = true;
	uint32_t const n_qubits = circuit.num_qubits();
	for (uint32_t a = 0; a < (1u << n); ++a) {
		for (uint32_t b = 0; b < (1u << n); ++b) {
			DynamicBitset<uint8_t> input(n_qubits, (b << n) + a);
			DynamicBitset<uint8_t> expected(n_qubits, (b << n) + a);
			expected[2*n] = (a < b);
			auto sim_pattern = simulate_classically(circuit, input);
			is_valid &= (sim_pattern == expected);
		}
	}
	return is_valid;
}

TEST_CASE("Less than (<)", "[lt][gen]")
{
	using namespace tweedledum;
	uint32_t n = 5;
	Circuit circuit = less_than(n);

	// to_qpic(std::cout, circuit);

	CHECK(validate_lt(circuit, n));
}
