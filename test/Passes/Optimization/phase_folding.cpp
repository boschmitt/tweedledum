/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Optimization/phase_folding.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"

#include "../check_unitary.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Trivial phase folding", "[phase_folding][optimization]")
{
    using namespace tweedledum;
    SECTION("Trivial 1 qubit") {
        Circuit circuit;
        Qubit q0 = circuit.create_qubit();
        circuit.apply_operator(Op::T(), {q0});
        circuit.apply_operator(Op::Tdg(), {q0});
        
        Circuit optimized = phase_folding(circuit);
        CHECK(optimized.size() == 0u);
        CHECK(check_unitary(circuit, optimized));
    }
    SECTION("Trivial 2 qubit swap") {
        Circuit circuit;
        Qubit q0 = circuit.create_qubit();
        Qubit q1 = circuit.create_qubit();
        circuit.apply_operator(Op::T(), {q0});
        circuit.apply_operator(Op::Swap(), {q1, q0});
        circuit.apply_operator(Op::Tdg(), {q1});
        
        Circuit optimized = phase_folding(circuit);
        CHECK(optimized.size() == 1u);
        CHECK(check_unitary(circuit, optimized));
    }
}
