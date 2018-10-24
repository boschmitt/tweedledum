/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <cstdlib>
#include <iostream>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <tweedledum/algorithms/synthesis/control_function.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/write_qpic.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;

class resource_counter {
public:
	void add_qubit()
	{
		q++;
	}

	uint32_t num_qubits()
	{
		return q;
	}

	void add_gate(gate_kinds_t, std::vector<uint32_t>, std::vector<uint32_t>)
	{
		t++;
	}

	void print()
	{
		std::cout << "qubit-count: " << q << '\n' << "gate-count: " << t << '\n';
	}

private:
	uint32_t q = 0;
	uint32_t t = 0;
};

int main(int argc, char** argv)
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto ntk = control_function_synthesis<netlist<mcmt_gate>>(tt);
	write_qpic(ntk, "dagstuhl.qpic");

	auto counter = control_function_synthesis<resource_counter>(tt);
	counter.print();
	return EXIT_SUCCESS;
}
