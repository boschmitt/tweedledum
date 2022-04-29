/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Circuit.h"

#include <catch.hpp>

TEST_CASE("Circuit qubits and cbits", "[circuit][ir]")
{
    using namespace tweedledum;
    Circuit circuit;
    CHECK(circuit.num_instructions() == 0u);
    CHECK(circuit.num_ancillae() == 0u);
    CHECK(circuit.num_qubits() == 0u);
    CHECK(circuit.num_cbits() == 0u);
    CHECK(circuit.global_phase() == 0.0);

    // Create a Qubit and a Cbit (No names)
    circuit.create_cbit();
    circuit.create_qubit();
    CHECK(circuit.num_cbits() == 1u);
    CHECK(circuit.num_qubits() == 1u);

    // Request ancilla
    Qubit a0 = circuit.request_ancilla();
    CHECK(circuit.num_qubits() == 2u);
    CHECK(circuit.num_ancillae() == 0u);
    CHECK(a0.uid() == 1);

    // Release ancilla and request it again
    circuit.release_ancilla(a0);
    CHECK(a0 == Qubit::invalid());
    CHECK(circuit.num_qubits() == 2u);
    CHECK(circuit.num_ancillae() == 1u);
    a0 = circuit.request_ancilla();
    CHECK(a0.uid() == 1);
    CHECK(circuit.num_qubits() == 2u);
    CHECK(circuit.num_ancillae() == 0u);

    // Create named qubit and cbit
    Cbit cbit = circuit.create_cbit("named_cbit");
    Qubit qubit = circuit.create_qubit("named_qubit");
    CHECK(circuit.num_cbits() == 2u);
    CHECK(circuit.num_qubits() == 3u);
    CHECK(circuit.num_ancillae() == 0u);

    // WireStorage checks
    CHECK(circuit.name(cbit) == "named_cbit");
    CHECK(circuit.name(qubit) == "named_qubit");
    CHECK(circuit.qubit(1) == cbit);
    CHECK(circuit.qubit(2) == qubit);
    CHECK(circuit.cbits() == std::vector<Cbit>({Cbit(0), cbit}));
    CHECK(circuit.qubits() == std::vector<Qubit>({Qubit(0), a0, qubit}));
}

class Dummy {
public:
    Dummy()
    {
        ++constructed;
    }

    Dummy(Dummy const& other)
    {
        ++copy_constructed;
    }

    Dummy(Dummy&& other) noexcept
    {
        ++move_constructed;
    }

    ~Dummy()
    {
        ++destructed;
    }

    static constexpr std::string_view kind()
    {
        return "dummy_optor";
    }

    static uint8_t constructed;
    static uint8_t copy_constructed;
    static uint8_t move_constructed;
    static uint8_t destructed;
};

uint8_t Dummy::constructed;
uint8_t Dummy::copy_constructed;
uint8_t Dummy::move_constructed;
uint8_t Dummy::destructed;

TEST_CASE("Circuit apply operator", "[circuit][ir]")
{
    using namespace tweedledum;
    Dummy::constructed = 0;
    Dummy::copy_constructed = 0;
    Dummy::move_constructed = 0;
    Dummy::destructed = 0;
    Circuit circuit;
    Cbit c0 = circuit.create_cbit();
    Qubit q0 = circuit.create_qubit();

    SECTION("Construct and move")
    {
        circuit.apply_operator(Dummy(), {q0}, {c0});
        CHECK(circuit.num_instructions() == 1u);
        CHECK(Dummy::constructed == 1u);
        CHECK(Dummy::copy_constructed == 0u);
        CHECK(Dummy::move_constructed == 1u);
        CHECK(Dummy::destructed == 1u);
    }
    SECTION("Construct and copy")
    {
        Dummy dummy;
        circuit.apply_operator(dummy, {q0}, {c0});
        CHECK(Dummy::constructed == 1u);
        CHECK(Dummy::copy_constructed == 1u);
        CHECK(Dummy::move_constructed == 0u);
        CHECK(Dummy::destructed == 0u);
        CHECK(circuit.instruction(InstRef(0)).is_a<Dummy>());
    }
    SECTION("Duplicate circuit")
    {
        Dummy dummy;
        circuit.apply_operator(dummy, {q0}, {c0});

        Circuit duplicate;
        duplicate.create_cbit();
        duplicate.create_qubit();
        circuit.foreach_instruction([&duplicate](Instruction const& inst) {
            duplicate.apply_operator(inst);
        });
        CHECK(Dummy::constructed == 1u);
        CHECK(Dummy::copy_constructed == 2u);
        CHECK(Dummy::move_constructed == 0u);
        CHECK(Dummy::destructed == 0u);
        CHECK(
          duplicate.instruction(InstRef(0)) == circuit.instruction(InstRef(0)));
    }
}
