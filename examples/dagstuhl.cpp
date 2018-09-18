/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#include <cstdlib>
#include <iostream>
#include <tweedledum/algorithms/synthesis/esop_based.hpp>
#include <tweedledum/io/write_qpic.hpp>
#include <tweedledum/networks/gates/mct_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>

using namespace tweedledum;

class resource_counter {
public:
	void allocate_qubit()
	{
		q++;
	}

	void add_multiple_controlled_target_gate(gate_kinds_t, uint32_t, uint32_t)
	{
		t++;
	}
	
	void print()
	{
		std::cout << "qubit-count: " << q << '\n'
		          << "gate-count: " << t << '\n'; 
	}

private:
	uint32_t q = 0;
	uint32_t t = 0;
};

int main(int argc, char** argv)
{
	netlist<mct_gate> ntk;
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	esop_based_synthesis(ntk, tt);
	write_qpic(ntk, "dagstuhl.qpic");

	resource_counter counter;
	esop_based_synthesis(counter, tt);
	counter.print();
	return EXIT_SUCCESS;
}
