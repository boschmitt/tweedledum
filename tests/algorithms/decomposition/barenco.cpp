/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/decomposition/barenco.hpp"

#include "tweedledum/algorithms/simulation/simulate_classically.hpp"
#include "tweedledum/algorithms/verification/unitary_verify.hpp"
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/wire.hpp"
#include "tweedledum/operations/wn32_op.hpp"

#include <algorithm>
#include <catch.hpp>
#include <vector>

using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Barenco decompostion", "[decomp][barenco]", (netlist), (wn32_op))
{
	SECTION("Without relative phase") {
		for (uint32_t i = 4u; i <= 8; ++i) {
			TestType original;
			std::vector<wire::id> qubits(i, wire::invalid_id);
			std::generate(qubits.begin(), qubits.end(),
			              [&]() { return original.create_qubit(); });

			original.create_op(gate_lib::ncx,
			                   std::vector(qubits.begin() + 1, qubits.end() - 1),
			                   std::vector({qubits.at(0)}));

			barenco_params params;
			params.use_ncrx = false;
			TestType decomposed = barenco_decomposition(original, params);
			CHECK(unitary_verify(original, decomposed));
		}
	}
	SECTION("With relative phase") {
		for (uint32_t i = 4u; i <= 8; ++i) {
			TestType original;
			std::vector<wire::id> qubits(i, wire::invalid_id);
			std::generate(qubits.begin(), qubits.end(),
			              [&]() { return original.create_qubit(); });

			original.create_op(gate_lib::ncx,
			                   std::vector(qubits.begin() + 1, qubits.end() - 1),
			                   std::vector({qubits.at(0)}));
			TestType decomposed = barenco_decomposition(original);
			CHECK(unitary_verify(original, decomposed));
		}
	}
}
