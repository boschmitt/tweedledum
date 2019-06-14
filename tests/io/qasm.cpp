/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <sstream>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/io/qasm.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/io_id.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Write simple network into qasm", "[qasm][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	TestType network;
	auto q0 = network.add_qubit();
	// network.add_cbit();
	auto q1 = network.add_qubit();
	// network.add_cbit();
	// network.add_cbit();
	auto q2 = network.add_qubit();
	std::vector<io_id> controls = {q0, q1};
	std::vector<io_id> target = {q2};
	network.add_gate(gate::mcx, controls, target);
	// CHECK(network.size() == 13);
	// CHECK(network.num_qubits() == 3);
	// CHECK(network.num_cbits() == 3);
	// CHECK(network.num_gates() == 1);

	std::ostringstream os;
	write_qasm(network, os);
	// CHECK(os.str() == "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[3];\ncreg c[3];\nccx q[0], q[1], q[2];\n");
	CHECK(os.str() == "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[3];\nccx q[0], q[1], q[2];\n");
}
