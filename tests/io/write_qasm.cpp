/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/write_qasm.hpp>
#include <tweedledum/networks/gdg_network.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE("Write simple MCMT into qasm", "[qasm]")
{
	using namespace tweedledum;
	gg_network<mcmt_gate> gg_net;
	gg_net.add_qubit();
	gg_net.add_qubit();
	gg_net.add_qubit();
	auto controls = std::vector<uint32_t>({0u, 1u});
	auto target = std::vector<uint32_t>({2u});
	gg_net.add_gate(gate_kinds_t::mcx, controls, target);
	CHECK(gg_net.size() == 7);
	CHECK(gg_net.num_qubits() == 3);
	CHECK(gg_net.num_gates() == 1);

	std::ostringstream os;
	write_qasm(gg_net, os);
	CHECK(os.str() == "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[3];\ncreg c[3];\nccx q[0], q[1], q[2];\n");
}
