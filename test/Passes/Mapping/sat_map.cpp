/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/sat_map.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Target/Device.h"

#include <catch.hpp>

TEST_CASE("sat_map test cases", "[sat_map][mapping]")
{
    using namespace tweedledum;
    Circuit circuit;
    SECTION("Empty circuit") {
        Device device = Device::path(circuit.num_qubits());
        auto mapped = sat_map(circuit, device);
        CHECK(mapped.size() == 0u);
        CHECK(mapped.num_wires() == 0u);
        CHECK(mapped.num_qubits() == 0u);
        CHECK(mapped.num_cbits() == 0u);
        // CHECK(mapped.num_operations() == 0u);
    }
    SECTION("Circuit with no instructions") {
        circuit.create_qubit();
        circuit.create_cbit();
        circuit.create_qubit();
        circuit.create_cbit();
        circuit.create_qubit();
        circuit.create_cbit();
        Device device = Device::path(circuit.num_qubits());
        auto mapped = sat_map(circuit, device);
        CHECK(mapped.size() == circuit.size());
        CHECK(mapped.num_wires() == circuit.num_wires());
        CHECK(mapped.num_qubits() == circuit.num_qubits());
        CHECK(mapped.num_cbits() == circuit.num_cbits());
        // CHECK(mapped.num_operations() == 0u);
    }
    SECTION("Simple circuit (SAT)") {
        Qubit q0 = circuit.create_qubit();
        circuit.create_cbit();
        Qubit q1 = circuit.create_qubit();
        circuit.create_cbit();
        Qubit q2 = circuit.create_qubit();
        circuit.create_cbit();

        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q2, q0});

        Device device = Device::path(circuit.num_qubits());
        auto mapped = sat_map(circuit, device);
        CHECK(mapped.size() == circuit.size());
    }
    SECTION("Simple circuit (UNSAT)") {
        Qubit q0 = circuit.create_qubit();
        circuit.create_cbit();
        Qubit q1 = circuit.create_qubit();
        circuit.create_cbit();
        Qubit q2 = circuit.create_qubit();
        circuit.create_cbit();

        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q1, q2});
        circuit.apply_operator(Op::X(), {q2, q0});

        Device device = Device::path(circuit.num_qubits());
        auto mapped = sat_map(circuit, device);
        CHECK(mapped.size() != circuit.size());
        CHECK(mapped.size() == 0);
    }
}
