/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/wire.hpp"


template<typename Network>
Network test_circuit_00()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();

	circuit.create_op(gate_lib::cx, q1, q0);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q2, q0);
	return circuit;
}

// Extend ZDD mapper paper example
template<typename Network>
Network test_circuit_01()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q3);
	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q3);
	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q3, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q3);

	return circuit;
}

// Extend ZDD mapper paper example #2
template<typename Network>
Network test_circuit_02()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	circuit.create_qubit();
	auto q5 = circuit.create_qubit();

	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q5);
	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);

	return circuit;
}

// Extend ZDD mapper paper example #3
template<typename Network>
Network test_circuit_03()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q3);
	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q3);
	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q3, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q3);
	circuit.create_op(gate_lib::cx, q3, q2);
	circuit.create_op(gate_lib::cx, q3, q1);
	circuit.create_op(gate_lib::cx, q3, q0);

	return circuit;
}

// Extend ZDD mapper paper example #3.5
template<typename Network>
Network test_circuit_04()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	wire::id q4 = circuit.create_qubit();
	wire::id q5 = circuit.create_qubit();
	wire::id q6 = circuit.create_qubit();
	wire::id q7 = circuit.create_qubit();
	wire::id q8 = circuit.create_qubit();
	wire::id q9 = circuit.create_qubit();

	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q5);
	circuit.create_op(gate_lib::cx, q9, q8);
	circuit.create_op(gate_lib::cx, q1, q5);
	circuit.create_op(gate_lib::cx, q4, q3);
	circuit.create_op(gate_lib::cx, q8, q7);
	circuit.create_op(gate_lib::cx, q6, q8);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q2, q5);
	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);

	return circuit;
}

// Extend ZDD mapper paper #4
template<typename Network>
Network test_circuit_05()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	wire::id q4 = circuit.create_qubit();
	wire::id q5 = circuit.create_qubit();
	wire::id q6 = circuit.create_qubit();
	wire::id q7 = circuit.create_qubit();

	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q1, q3);
	circuit.create_op(gate_lib::cx, q4, q5);
	circuit.create_op(gate_lib::cx, q5, q6);
	circuit.create_op(gate_lib::cx, q5, q7);

	return circuit;
}

template<typename Network>
Network test_circuit_06()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	wire::id q4 = circuit.create_qubit();
	wire::id q5 = circuit.create_qubit();

	circuit.create_op(gate_lib::cx, q0, q2);
	circuit.create_op(gate_lib::cx, q2, q1);
	circuit.create_op(gate_lib::cx, q0, q4);
	circuit.create_op(gate_lib::cx, q3, q0);
	circuit.create_op(gate_lib::cx, q0, q5);

	return circuit;
}

template<typename Network>
Network test_circuit_07()
{
	using namespace tweedledum;
	Network circuit;
	wire::id q0 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q1 = circuit.create_qubit();
	wire::id q2 = circuit.create_qubit();
	circuit.create_cbit();
	wire::id q3 = circuit.create_qubit();
	circuit.create_cbit();

	wire::id q4 = circuit.create_qubit();

	circuit.create_op(gate_lib::cx, q0, q1);
	circuit.create_op(gate_lib::cx, q1, q2);
	circuit.create_op(gate_lib::cx, q2, q3);
	circuit.create_op(gate_lib::cx, q3, q4);
	circuit.create_op(gate_lib::cx, q0, q4);

	return circuit;
}