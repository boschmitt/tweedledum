/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Instruction.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/All.h"
#include "tweedledum/Utils/Numbers.h"

#include <catch.hpp>

TEST_CASE("Check single-target, one-qubit adjointness", "[instruction][ir]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.apply_operator(Op::H(), {q0});
    circuit.apply_operator(Op::P(numbers::pi_div_4), {q0});
    circuit.apply_operator(Op::P(numbers::pi_div_2), {q0});
    circuit.apply_operator(Op::P(numbers::pi), {q0});
    circuit.apply_operator(Op::Rx(numbers::pi_div_4), {q0});
    circuit.apply_operator(Op::Rx(numbers::pi_div_2), {q0});
    circuit.apply_operator(Op::Rx(numbers::pi), {q0});
    circuit.apply_operator(Op::Ry(numbers::pi_div_4), {q0});
    circuit.apply_operator(Op::Ry(numbers::pi_div_2), {q0});
    circuit.apply_operator(Op::Ry(numbers::pi), {q0});
    circuit.apply_operator(Op::Rz(numbers::pi_div_4), {q0});
    circuit.apply_operator(Op::Rz(numbers::pi_div_2), {q0});
    circuit.apply_operator(Op::Rz(numbers::pi), {q0});
    circuit.apply_operator(Op::S(), {q0});
    circuit.apply_operator(Op::T(), {q0});
    circuit.apply_operator(Op::X(), {q0});
    circuit.apply_operator(Op::Y(), {q0});
    circuit.apply_operator(Op::Z(), {q0});

    SECTION("Adjoints")
    {
        // Create a circuit with adjoints
        Circuit adjoints;
        adjoints.create_qubit();
        circuit.foreach_instruction([&adjoints](Instruction const& inst) {
            std::optional<Operator> adj = inst.adjoint();
            REQUIRE(adj);
            adjoints.apply_operator(*adj, inst.qubits());
        });
        REQUIRE(circuit.size() == adjoints.size());

        // Check for correctness
        for (uint32_t i = 0; i < circuit.size(); ++i) {
            InstRef const ref(i);
            Instruction const& inst0 = circuit.instruction(ref);
            Instruction const& inst1 = adjoints.instruction(ref);
            CHECK(inst0.is_adjoint(inst1));
            CHECK(inst1.is_adjoint(inst0));
            for (uint32_t j = i + 1; j < circuit.size(); ++j) {
                Instruction const& inst = circuit.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
            for (uint32_t j = i + 1; j < adjoints.size(); ++j) {
                Instruction const& inst = adjoints.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
        }
    }
    SECTION("Not adjoints (different qubits)")
    {
        // Create a circuit with adjoints but on other qubits
        Circuit non_adjoints;
        non_adjoints.create_qubit();
        Qubit q1 = non_adjoints.create_qubit();
        circuit.foreach_instruction(
          [&non_adjoints, q1](Instruction const& inst) {
              std::optional<Operator> adj = inst.adjoint();
              non_adjoints.apply_operator(*adj, {q1});
          });
        REQUIRE(circuit.size() == non_adjoints.size());

        // Check for correctness
        for (uint32_t i = 0; i < circuit.size(); ++i) {
            InstRef const ref(i);
            Instruction const& inst0 = circuit.instruction(ref);
            Instruction const& inst1 = non_adjoints.instruction(ref);
            CHECK_FALSE(inst0.is_adjoint(inst1));
            CHECK_FALSE(inst1.is_adjoint(inst0));
            for (uint32_t j = i + 1; j < non_adjoints.size(); ++j) {
                Instruction const& inst = non_adjoints.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
        }
    }
}

TEST_CASE("Check single-target, two-qubit adjointness", "[instruction][ir]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    circuit.apply_operator(Op::H(), {q0});
    circuit.apply_operator(Op::P(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::P(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::P(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::Rx(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::Rx(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::Rx(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::Ry(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::Ry(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::Ry(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::Rz(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::Rz(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::Rz(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::S(), {q0, q1});
    circuit.apply_operator(Op::T(), {q0, q1});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::Y(), {q0, q1});
    circuit.apply_operator(Op::Z(), {q0, q1});

    SECTION("Adjoints")
    {
        // Create a circuit with adjoints
        Circuit adjoints;
        adjoints.create_qubit();
        adjoints.create_qubit();
        circuit.foreach_instruction([&adjoints](Instruction const& inst) {
            std::optional<Operator> adj = inst.adjoint();
            REQUIRE(adj);
            adjoints.apply_operator(*adj, inst.qubits());
        });
        REQUIRE(circuit.size() == adjoints.size());

        // Check for correctness
        for (uint32_t i = 0; i < circuit.size(); ++i) {
            InstRef const ref(i);
            Instruction const& inst0 = circuit.instruction(ref);
            Instruction const& inst1 = adjoints.instruction(ref);
            CHECK(inst0.is_adjoint(inst1));
            CHECK(inst1.is_adjoint(inst0));
            for (uint32_t j = i + 1; j < circuit.size(); ++j) {
                Instruction const& inst = circuit.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
            for (uint32_t j = i + 1; j < adjoints.size(); ++j) {
                Instruction const& inst = adjoints.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
        }
    }
    SECTION("Not adjoints (same qubits, different order)")
    {
        // Create a circuit with adjoints but on other qubits
        Circuit non_adjoints;
        non_adjoints.create_qubit();
        non_adjoints.create_qubit();
        circuit.foreach_instruction([&](Instruction const& inst) {
            std::optional<Operator> adj = inst.adjoint();
            non_adjoints.apply_operator(*adj, {q1, q0});
        });
        REQUIRE(circuit.size() == non_adjoints.size());

        // Check for correctness
        for (uint32_t i = 0; i < circuit.size(); ++i) {
            InstRef const ref(i);
            Instruction const& inst0 = circuit.instruction(ref);
            Instruction const& inst1 = non_adjoints.instruction(ref);
            CHECK_FALSE(inst0.is_adjoint(inst1));
            CHECK_FALSE(inst1.is_adjoint(inst0));
            for (uint32_t j = i + 1; j < non_adjoints.size(); ++j) {
                Instruction const& inst = non_adjoints.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
        }
    }
}

TEST_CASE("Check two-target, two-qubit adjointness", "[instruction][ir]")
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    circuit.apply_operator(Op::Rxx(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::Rxx(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::Rxx(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::Ryy(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::Ryy(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::Ryy(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::Rzz(numbers::pi_div_4), {q0, q1});
    circuit.apply_operator(Op::Rzz(numbers::pi_div_2), {q0, q1});
    circuit.apply_operator(Op::Rzz(numbers::pi), {q0, q1});
    circuit.apply_operator(Op::Swap(), {q0, q1});

    SECTION("Adjoints")
    {
        // Create a circuit with adjoints
        Circuit adjoints;
        adjoints.create_qubit();
        adjoints.create_qubit();
        circuit.foreach_instruction([&adjoints](Instruction const& inst) {
            std::optional<Operator> adj = inst.adjoint();
            REQUIRE(adj);
            adjoints.apply_operator(*adj, inst.qubits());
        });
        REQUIRE(circuit.size() == adjoints.size());

        // Check for correctness
        for (uint32_t i = 0; i < circuit.size(); ++i) {
            InstRef const ref(i);
            Instruction const& inst0 = circuit.instruction(ref);
            Instruction const& inst1 = adjoints.instruction(ref);
            CHECK(inst0.is_adjoint(inst1));
            CHECK(inst1.is_adjoint(inst0));
            for (uint32_t j = i + 1; j < circuit.size(); ++j) {
                Instruction const& inst = circuit.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
            for (uint32_t j = i + 1; j < adjoints.size(); ++j) {
                Instruction const& inst = adjoints.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
        }
    }
    SECTION("Not adjoints (same qubits, different order)")
    {
        // Create a circuit with adjoints but on other qubits
        Circuit non_adjoints;
        non_adjoints.create_qubit();
        non_adjoints.create_qubit();
        circuit.foreach_instruction([&](Instruction const& inst) {
            std::optional<Operator> adj = inst.adjoint();
            non_adjoints.apply_operator(*adj, {q1, q0});
        });
        REQUIRE(circuit.size() == non_adjoints.size());

        // Check for correctness
        for (uint32_t i = 0; i < circuit.size(); ++i) {
            InstRef const ref(i);
            Instruction const& inst0 = circuit.instruction(ref);
            Instruction const& inst1 = non_adjoints.instruction(ref);
            CHECK_FALSE(inst0.is_adjoint(inst1));
            CHECK_FALSE(inst1.is_adjoint(inst0));
            for (uint32_t j = i + 1; j < non_adjoints.size(); ++j) {
                Instruction const& inst = non_adjoints.instruction(InstRef(j));
                CHECK_FALSE(inst0.is_adjoint(inst));
                CHECK_FALSE(inst.is_adjoint(inst0));
            }
        }
    }
}
