/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <iostream>
#include <tweedledum/gates/gate.hpp>
#include <tweedledum/io/write_utf8.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/operations/w3_op.hpp>

int main(int argc, char** argv)
{
	using namespace tweedledum;

	using namespace tweedledum;
	netlist<w3_op> circuit;
	wire::id q0 = circuit.create_qubit("q0");
	wire::id c0 = circuit.create_cbit("c0");
	circuit.create_op(gate_lib::h, q0);
	circuit.create_op(gate_lib::measure_z, q0, c0);
	write_utf8(circuit);
}
