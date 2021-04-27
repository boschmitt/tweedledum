/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Analysis/count_operators.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Count operators", "[cout_operators][analysis]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.apply_operator(Op::T(), {q0});
    circuit.apply_operator(Op::Tdg(), {q0});
    circuit.apply_operator(Op::X(), {q0});
    circuit.apply_operator(Op::X(), {q1});
    circuit.apply_operator(Op::X(), {q2});
    circuit.apply_operator(Op::X(), {q1, q0});
    circuit.apply_operator(Op::X(), {q2, q1, q0});
    auto counters = count_operators(circuit);
    CHECK(counters["t"] == 1u);
    CHECK(counters["tdg"] == 1u);
    CHECK(counters["x"] == 3u);
    CHECK(counters["(1c)x"] == 1u);
    CHECK(counters["(2c)x"] == 1u);
}
