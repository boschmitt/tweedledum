/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Qubit.h"
#include "tweedledum/Operators/Standard/X.h"

inline tweedledum::Circuit test_circuit_00()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();

    circuit.apply_operator(Op::X(), {q1, q0});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q2, q0});
    return circuit;
}

inline tweedledum::Circuit test_circuit_01()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();

    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q3});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q3});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q3, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q3});

    return circuit;
}

// Extend ZDD mapper paper example #2
inline tweedledum::Circuit test_circuit_02()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();
    circuit.create_qubit();
    auto q5 = circuit.create_qubit();

    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q5});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});

    return circuit;
}

// Extend ZDD mapper paper example #3
inline tweedledum::Circuit test_circuit_03()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();

    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q3});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q3});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q3, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q3});
    circuit.apply_operator(Op::X(), {q3, q2});
    circuit.apply_operator(Op::X(), {q3, q1});
    circuit.apply_operator(Op::X(), {q3, q0});

    return circuit;
}

// Extend ZDD mapper paper example #3.5
inline tweedledum::Circuit test_circuit_04()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();

    Qubit q4 = circuit.create_qubit();
    Qubit q5 = circuit.create_qubit();
    Qubit q6 = circuit.create_qubit();
    Qubit q7 = circuit.create_qubit();
    Qubit q8 = circuit.create_qubit();
    Qubit q9 = circuit.create_qubit();

    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q5});
    circuit.apply_operator(Op::X(), {q9, q8});
    circuit.apply_operator(Op::X(), {q1, q5});
    circuit.apply_operator(Op::X(), {q4, q3});
    circuit.apply_operator(Op::X(), {q8, q7});
    circuit.apply_operator(Op::X(), {q6, q8});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q2, q5});
    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});

    return circuit;
}

// Extend ZDD mapper paper #4
inline tweedledum::Circuit test_circuit_05()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();

    Qubit q4 = circuit.create_qubit();
    Qubit q5 = circuit.create_qubit();
    Qubit q6 = circuit.create_qubit();
    Qubit q7 = circuit.create_qubit();

    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q1, q3});
    circuit.apply_operator(Op::X(), {q4, q5});
    circuit.apply_operator(Op::X(), {q5, q6});
    circuit.apply_operator(Op::X(), {q5, q7});

    return circuit;
}

inline tweedledum::Circuit test_circuit_06()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();

    Qubit q4 = circuit.create_qubit();
    Qubit q5 = circuit.create_qubit();

    circuit.apply_operator(Op::X(), {q0, q2});
    circuit.apply_operator(Op::X(), {q2, q1});
    circuit.apply_operator(Op::X(), {q0, q4});
    circuit.apply_operator(Op::X(), {q3, q0});
    circuit.apply_operator(Op::X(), {q0, q5});

    return circuit;
}

inline tweedledum::Circuit test_circuit_07()
{
    using namespace tweedledum;
    Circuit circuit;
    Qubit q0 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q1 = circuit.create_qubit();
    Qubit q2 = circuit.create_qubit();
    circuit.create_cbit();
    Qubit q3 = circuit.create_qubit();
    circuit.create_cbit();

    Qubit q4 = circuit.create_qubit();

    circuit.apply_operator(Op::X(), {q0, q1});
    circuit.apply_operator(Op::X(), {q1, q2});
    circuit.apply_operator(Op::X(), {q2, q3});
    circuit.apply_operator(Op::X(), {q3, q4});
    circuit.apply_operator(Op::X(), {q0, q4});

    return circuit;
}
