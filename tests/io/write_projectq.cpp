/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/qubit.hpp>

TEST_CASE("Write simple MCMT into projectq", "[projectq]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> mcmt_netlist;
	auto q0 = mcmt_netlist.add_qubit();
	auto q1 = mcmt_netlist.add_qubit();
	auto q2 = mcmt_netlist.add_qubit();

	std::vector<qubit_id> controls = {q0, q1};
	std::vector<qubit_id> target = {q2};
	mcmt_netlist.add_gate(gate::mcx, controls, target);

	CHECK(mcmt_netlist.size() == 7);
	CHECK(mcmt_netlist.num_qubits() == 3);
	CHECK(mcmt_netlist.num_gates() == 1);

	std::ostringstream os;
	write_projectq(mcmt_netlist, os);
	CHECK(os.str() == "C(All(X), 2) | ([qs[0], qs[1]], [qs[2]])\n");
}

TEST_CASE("Write simple MCMT with nagated controls into projectq", "[projectq]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> mcmt_netlist;
	auto q0 = mcmt_netlist.add_qubit();
	auto q1 = mcmt_netlist.add_qubit();
	auto q2 = mcmt_netlist.add_qubit();

	std::vector<qubit_id> controls = {!q0, !q1};
	std::vector<qubit_id> target = {q2};
	mcmt_netlist.add_gate(gate::mcx, controls, target);

	CHECK(mcmt_netlist.size() == 7);
	CHECK(mcmt_netlist.num_qubits() == 3);
	CHECK(mcmt_netlist.num_gates() == 1);

	std::ostringstream os;
	write_projectq(mcmt_netlist, os);
	CHECK(os.str() == "X | qs[0], qs[1]\nC(All(X), 2) | ([qs[0], qs[1]], [qs[2]])\nX | qs[0], qs[1]\n");
}
