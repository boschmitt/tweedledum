/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Placer/ApprxSatPlacer.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Qubit.h"
#include "tweedledum/Operators/Standard/X.h"
#include "tweedledum/Target/Device.h"

#include <catch.hpp>

TEST_CASE("ApprxSatPlacer test cases", "[ApprxSatPlacer][mapping]")
{
    using namespace tweedledum;
    Circuit circuit;
    // SECTION("Empty circuit") {
    //     Device device = Device::path(circuit.num_qubits());
    //     auto placement = apprx_sat_place(device, circuit);
    //     CHECK_FALSE(placement);
    // }
    SECTION("Circuit with no instructions")
    {
        circuit.create_qubit();
        circuit.create_cbit();
        circuit.create_qubit();
        circuit.create_cbit();
        circuit.create_qubit();
        circuit.create_cbit();
        Device device = Device::path(circuit.num_qubits());
        auto placement = apprx_sat_place(device, circuit);
        CHECK(placement);
    }
    SECTION("Simple circuit (SAT)")
    {
        Qubit q0 = circuit.create_qubit();
        circuit.create_cbit();
        Qubit q1 = circuit.create_qubit();
        circuit.create_cbit();
        Qubit q2 = circuit.create_qubit();
        circuit.create_cbit();

        circuit.apply_operator(Op::X(), {q1, q0});
        circuit.apply_operator(Op::X(), {q2, q0});

        Device device = Device::path(circuit.num_qubits());
        auto placement = apprx_sat_place(device, circuit);
        CHECK(placement);
    }
    SECTION("Simple circuit (UNSAT)")
    {
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
        auto placement = apprx_sat_place(device, circuit);
        CHECK(placement);
    }
}
