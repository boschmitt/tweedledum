/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Analysis/compute_cuts.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Operators/Standard/Measure.h"
#include "tweedledum/Utils/Cut.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Compute intructions cuts with width 2", "[compute_cuts][analysis]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    SECTION("One-qubit instructions on different qubits")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, {InstRef(0), InstRef(1)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("One two-qubit instruction")
    {
        circuit.apply_operator(Op::X(), {q1, q0});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, InstRef(0)}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("One cut - mix of two-qubit and one-qubit instrunctions 0")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q1, q0});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, {InstRef(0), InstRef(1), InstRef(2)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("One cut - mix of two-qubit and one-qubit instrunctions 1")
    {
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, {InstRef(0), InstRef(1), InstRef(2)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("One cut - mix of two-qubit and one-qubit instrunctions 2")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q1});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, {InstRef(0), InstRef(1), InstRef(2)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("Two-qubit instruction that share only one qubit")
    {
        Qubit q2 = circuit.create_qubit();
        circuit.apply_operator(Op::X(), {q0, q1});
        circuit.apply_operator(Op::X(), {q2, q1});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, {InstRef(0)}},
            Cut{{q1, q2}, {}, {InstRef(1)}},
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("Cut interrupted by a instrunction that can't be added")
    {
        Cbit c = circuit.create_cbit();
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::Measure(), {q1}, {c});
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q1, q0});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {}, {InstRef(0), InstRef(1), InstRef(2), InstRef(4)}},
            Cut{{q1}, {c}, {InstRef(3)}},
            Cut{{q0, q1}, {}, {InstRef(5), InstRef(6)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("One-qubit instructions on different qubits, same cbit")
    {
        Cbit c = circuit.create_cbit();
        circuit.apply_operator(Op::X(), {q0}, {c});
        circuit.apply_operator(Op::X(), {q1}, {c});
        std::vector<Cut> expected = {
            Cut{{q0, q1}, {c}, {InstRef(0), InstRef(1)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
    SECTION("One-qubit instructions on different qubits, different cbits")
    {
        Cbit c0 = circuit.create_cbit();
        Cbit c1 = circuit.create_cbit();
        circuit.apply_operator(Op::X(), {q0}, {c0});
        circuit.apply_operator(Op::X(), {q1}, {c1});
        std::vector<Cut> expected = {
            Cut{{q0}, {c0}, {InstRef(0)}},
            Cut{{q1}, {c1}, {InstRef(1)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit);
        CHECK(expected == computed);
    }
}

TEST_CASE("Compute intructions cuts with width 3", "[compute_cuts][analysis]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    SECTION("One-qubit instructions on different qubits")
    {
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1});
        circuit.apply_operator(Op::X(), {q2});
        std::vector<Cut> expected = {
            Cut{{q0, q1, q2}, {}, {InstRef(0), InstRef(1), InstRef(2)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit, 3u);
        CHECK(expected == computed);
    }
    SECTION("One-qubit instructions on different qubits")
    {
        Qubit q3 = circuit.create_qubit();
        circuit.apply_operator(Op::X(), {q0});
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q1, q0, q2});
        circuit.apply_operator(Op::X(), {q1, q3});
        circuit.apply_operator(Op::X(), {q0});
        std::vector<Cut> expected = {
            Cut{{q0, q1, q2}, {}, {InstRef(0), InstRef(1), InstRef(2), InstRef(4)}},
            Cut{{q1, q3}, {}, {InstRef(3)}}
        };
        std::vector<Cut> computed = compute_cuts(circuit, 3u);
        CHECK(expected == computed);
    }
}
