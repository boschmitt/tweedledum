/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <iostream>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/gg_network.hpp>

int main(int argc, char** argv)
{
	(void) argc;
	(void) argv;
	using namespace tweedledum;
	gg_network<io3_gate> network;
	network.add_qubit("q0");
	network.add_qubit("q1");
	network.add_cbit("c0");
	network.add_cbit("c1");

	network.add_gate(gate::hadamard, "q0");
	network.add_gate(gate::cx, "q0", "q1");
	network.add_gate(gate::measurement, "q0", "c0");
	network.add_gate(gate::measurement, "q1", "c1");

	std::cout << "Hello world!\n";
	write_unicode(network);
}
