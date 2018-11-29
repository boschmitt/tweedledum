/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <iostream>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;
	using namespace tweedledum;
	netlist<mcst_gate> network;
	network.add_qubit("q0");
	network.add_qubit("q1");

	network.add_gate(gate_set::hadamard, "q0");
	network.add_gate(gate_set::cx, "q0", "q1");
	std::cout << "Hello world!\n";
}
