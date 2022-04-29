/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Qubit.h"
#include "tweedledum/Operators/All.h"

inline tweedledum::Circuit toffoli()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.apply_operator(Op::H(), {q2});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::Tdg(), {q2});
    circuit.apply_operator(Op::X(), {q0, q2});
    circuit.apply_operator(Op::T(), {q2});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::Tdg(), {q2});
    circuit.apply_operator(Op::X(), {q0, q2});
    circuit.apply_operator(Op::T(), {q1});
    circuit.apply_operator(Op::T(), {q2});
    circuit.apply_operator(Op::H(), {q2});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::T(), {q0});
    circuit.apply_operator(Op::Tdg(), {q1});
    circuit.apply_operator(Op::X(), {q0, q1});
    return circuit;
}

inline tweedledum::Circuit graph_coloring_init()
{
    constexpr double theta = 1.9106332362490184;
    using namespace tweedledum;
    Circuit circuit;
    std::vector<Qubit> qubits;
    for (uint32_t i = 0; i < 8; ++i) {
        qubits.push_back(circuit.create_qubit());
    }
    for (uint32_t i = 0u; i < 8u; i += 2u) {
        circuit.apply_operator(Op::Ry(theta), {qubits[i]});
        circuit.apply_operator(Op::H(), {qubits[i], qubits[i + 1]});
        circuit.apply_operator(Op::X(), {qubits[i + 1]});
    }
    return circuit;
}

inline tweedledum::Circuit ibm_contest2019_init()
{
    constexpr double theta = 1.9106332362490184;
    using namespace tweedledum;
    Circuit circuit;
    std::vector<Qubit> qubits;
    for (uint32_t i = 0; i < 14; ++i) {
        qubits.push_back(circuit.create_qubit());
    }
    // district0
    circuit.apply_operator(Op::Ry(theta), {qubits[0]});
    circuit.apply_operator(Op::H(), {qubits[0], qubits[1]});
    circuit.apply_operator(Op::X(), {qubits[1]});

    // district1
    circuit.apply_operator(Op::Ry(theta), {qubits[2]});
    circuit.apply_operator(Op::H(), {qubits[2], qubits[3]});

    // district2
    circuit.apply_operator(Op::H(), {qubits[4]});
    circuit.apply_operator(Op::X(), {qubits[5]});

    // district3
    circuit.apply_operator(Op::Ry(theta), {qubits[6]});
    circuit.apply_operator(Op::H(), {qubits[6], qubits[7]});
    circuit.apply_operator(Op::X(), {qubits[7]});

    // district4 B
    circuit.apply_operator(Op::Ry(theta), {qubits[8]});
    circuit.apply_operator(Op::H(), {qubits[8], qubits[9]});

    // district5
    circuit.apply_operator(Op::Ry(theta), {qubits[10]});
    circuit.apply_operator(Op::H(), {qubits[10], qubits[11]});
    circuit.apply_operator(Op::X(), {qubits[10]});

    // district6
    circuit.apply_operator(Op::Ry(theta), {qubits[12]});
    circuit.apply_operator(Op::H(), {qubits[12], qubits[13]});
    circuit.apply_operator(Op::X(), {qubits[12]});
    return circuit;
}
