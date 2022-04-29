/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Analysis/compute_critical_paths.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/All.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Compute critical path(s)", "[compute_critical_paths][analysis]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    SECTION("One qubit")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q0});
        auto paths = compute_critical_paths(circuit);
        std::vector<InstRef> expected = {InstRef(0), InstRef(1), InstRef(2)};
        CHECK(paths.size() == 1u);
        CHECK(paths.at(0).size() == 3u);
        CHECK(paths.at(0) == expected);
    }
    Qubit q1 = circuit.create_qubit();
    SECTION("Two qubits (0)")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        auto paths = compute_critical_paths(circuit);
        std::vector<InstRef> expected_0 = {InstRef(0), InstRef(2), InstRef(4)};
        std::vector<InstRef> expected_1 = {InstRef(1), InstRef(3), InstRef(5)};
        CHECK(paths.size() == 2u);
        CHECK(paths.at(0).size() == 3u);
        CHECK(paths.at(1).size() == 3u);
        CHECK(paths.at(0) == expected_0);
        CHECK(paths.at(1) == expected_1);
    }
    SECTION("Two qubits (1)")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q1});
        auto paths = compute_critical_paths(circuit);
        std::vector<InstRef> expected = {
          InstRef(0), InstRef(1), InstRef(3), InstRef(4), InstRef(5)};
        CHECK(paths.size() == 1u);
        CHECK(paths.at(0).size() == 5u);
        CHECK(paths.at(0) == expected);
    }
}
