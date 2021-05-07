/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Analysis/compute_asap_layers.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/All.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Compute intructions' ASAP layer", "[compute_asap_layers][analysis]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    SECTION("Two qubits (0)")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q1});
        std::vector<uint32_t> expected = {0, 1, 2, 2, 3};
        std::vector<uint32_t> asap_layers = compute_asap_layers(circuit);
        CHECK(expected == asap_layers);
    }
    Qubit q2 = circuit.create_qubit();
    SECTION("Three qubits (0)")
    {
        circuit.apply_operator(Op::X(), {q0}); // Layer 0
        circuit.apply_operator(Op::X(), {q0}); // Layer 1
        circuit.apply_operator(Op::X(), {q1, q0}); // Layer 2
        circuit.apply_operator(Op::X(), {q1}); // Layer 3
        circuit.apply_operator(Op::X(), {q2}); // Layer 0
        circuit.apply_operator(Op::X(), {q2}); // Layer 1
        circuit.apply_operator(Op::X(), {q2, q0}); // Layer 3
        std::vector<uint32_t> expected = {0, 1, 2, 3, 0, 1, 3};
        std::vector<uint32_t> asap_layers = compute_asap_layers(circuit);
        CHECK(expected == asap_layers);
    }
}
