/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/qubit.hpp>

TEST_CASE("Write simple MCMT netlist into quil", "[quil]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> mcmt_netlist;
	mcmt_netlist.add_qubit();
	mcmt_netlist.add_qubit();
	mcmt_netlist.add_qubit();
	std::vector<qubit_id> controls = {qubit_id(0), qubit_id(1)};
	std::vector<qubit_id> target = {qubit_id(2)};
	mcmt_netlist.add_gate(gate::mcx, controls, target);
	CHECK(mcmt_netlist.size() == 7);
	CHECK(mcmt_netlist.num_qubits() == 3);
	CHECK(mcmt_netlist.num_gates() == 1);

	std::ostringstream os;
	write_quil(mcmt_netlist, os);
	CHECK(os.str() == "CCNOT 0 1 2\n");
}