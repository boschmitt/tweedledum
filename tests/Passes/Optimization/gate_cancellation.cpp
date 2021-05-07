/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Optimization/gate_cancellation.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"
#include "tweedledum/Passes/Utility/inverse.h"

#include "../check_unitary.h"
#include "../test_circuits.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Trivial gate cancellation", "[gate_cancellation][optimization]")
{
    Circuit circuit;
    Qubit const q0 = circuit.create_qubit();
    Qubit const q1 = circuit.create_qubit();
    SECTION("Single qubit operators")
    {
        circuit.apply_operator(Op::H(), {q0});
        circuit.apply_operator(Op::H(), {q0});
        circuit.apply_operator(Op::H(), {q1});
        circuit.apply_operator(Op::T(), {q1});
        circuit.apply_operator(Op::Tdg(), {q1});
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 1);
        CHECK(check_unitary(circuit, optimized));
    }
    SECTION("Two qubit X operator (0)")
    {
        circuit.apply_operator(Op::X(), {q0, q1});
        circuit.apply_operator(Op::X(), {q1, q0});
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 2);
        CHECK(check_unitary(circuit, optimized));
    }
    SECTION("Two qubit X operator (1)")
    {
        circuit.apply_operator(Op::X(), {q0, q1});
        circuit.apply_operator(Op::X(), {q0, q1});
        circuit.apply_operator(Op::X(), {q1, q0});
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 1);
        CHECK(check_unitary(circuit, optimized));
    }
    SECTION("Two qubit X operator (2)")
    {
        Qubit const q2 = circuit.create_qubit();
        circuit.apply_operator(Op::X(), {q0, q2});
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q0, q2});
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0);
        CHECK(check_unitary(circuit, optimized));
    }
}

TEMPLATE_TEST_CASE("Even Sequences (self-adjoint)",
  "[gate_cancellation][optimization]", Op::H, Op::X, Op::Y, Op::Z)
{
    Circuit circuit;
    Qubit const q0 = circuit.create_qubit();
    SECTION("One qubit")
    {
        for (uint32_t i = 0u; i < 1024u; ++i) {
            circuit.apply_operator(TestType(), {q0});
        }
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0);
    }
    Qubit const q1 = circuit.create_qubit();
    SECTION("Controlled")
    {
        for (uint32_t i = 0u; i < 1024u; ++i) {
            circuit.apply_operator(TestType(), {q1, q0});
        }
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0);
    }
    Qubit const q2 = circuit.create_qubit();
    SECTION("Multiple controls")
    {
        for (uint32_t i = 0u; i < 1024u; ++i) {
            circuit.apply_operator(TestType(), {q1, q2, q0});
        }
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0);
    }
}

TEMPLATE_TEST_CASE("Odd Sequences (self-adjoint)",
  "[gate_cancellation][optimization]", Op::H, Op::X, Op::Y, Op::Z)
{
    Circuit circuit;
    Qubit const q0 = circuit.create_qubit();
    SECTION("One qubit")
    {
        for (uint32_t i = 0u; i < 1023u; ++i) {
            circuit.apply_operator(TestType(), {q0});
        }
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 1);
    }
    Qubit const q1 = circuit.create_qubit();
    SECTION("Controlled")
    {
        for (uint32_t i = 0u; i < 1023; ++i) {
            circuit.apply_operator(TestType(), {q1, q0});
        }
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 1);
    }
    Qubit const q2 = circuit.create_qubit();
    SECTION("Multiple controls")
    {
        for (uint32_t i = 0u; i < 1023u; ++i) {
            circuit.apply_operator(TestType(), {q1, q2, q0});
        }
        auto optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 1);
    }
}

TEST_CASE("Inverted circuits.", "[gate_cancellation][optimization]")
{
    using namespace tweedledum;
    SECTION("Toffoli operator")
    {
        Circuit circuit = toffoli();
        std::optional<Circuit> adjoint = inverse(circuit);
        CHECK(adjoint);
        circuit.append(*adjoint, circuit.qubits(), circuit.cbits());
        Circuit optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0u);
    }
    SECTION("Graph coloring init")
    {
        Circuit circuit = graph_coloring_init();
        std::optional<Circuit> adjoint = inverse(circuit);
        CHECK(adjoint);
        circuit.append(*adjoint, circuit.qubits(), circuit.cbits());
        Circuit optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0u);
    }
    SECTION("IBM Contest 2019 init")
    {
        Circuit circuit = ibm_contest2019_init();
        std::optional<Circuit> adjoint = inverse(circuit);
        CHECK(adjoint);
        circuit.append(*adjoint, circuit.qubits(), circuit.cbits());
        Circuit optimized = gate_cancellation(circuit);
        CHECK(optimized.size() == 0u);
    }
}
