/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/io/qasm.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"

#include <catch.hpp>
#include <sstream>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("QASM Reader", "[qasm][template]", (netlist, op_dag), (w3_op, wn32_op))
{
	TestType network;
	SECTION("Empyt buffer") {
		std::string qasm = "";
		network = read_qasm_from_buffer<TestType>(qasm);
		CHECK(network.size() == 0u);
	}
	SECTION("Empyt network") {
		std::string qasm = "OPENQASM 2.0;\n"
		                   "include \"qelib1.inc\";\n";
		network = read_qasm_from_buffer<TestType>(qasm);
		CHECK(network.size() == 0u);
	}
	SECTION("Network without operations") {
		std::string qasm = "OPENQASM 2.0;\n"
		                   "include \"qelib1.inc\";\n"
		                   "qreg q[32];";
		network = read_qasm_from_buffer<TestType>(qasm);
		CHECK(network.size() == 32u);
		CHECK(network.num_wires() == 32u);
		CHECK(network.num_qubits() == 32u);
		CHECK(network.num_cbits() == 0u);
		network.foreach_wire([](wire_id wire, std::string const& label) {
			std::string name = fmt::format("q_{}", wire.id());
			CHECK(label == name);
		}); 
	}
}

TEMPLATE_PRODUCT_TEST_CASE("QASM Writer", "[qasm][template]", (netlist, op_dag), (w3_op, wn32_op))
{
	std::vector<gate> one_qubit = {gate_lib::i, gate_lib::h,   gate_lib::x,
	                               gate_lib::y, gate_lib::z,   gate_lib::s,
	                               gate_lib::t, gate_lib::sdg, gate_lib::tdg};
	std::vector<gate> two_qubit = {gate_lib::cx, gate_lib::cy, gate_lib::cz, gate_lib::swap};
	std::vector<gate> three_qubit = {gate_lib::ncx};

	TestType network;
	wire_id q0 = network.create_qubit();
	wire_id q1 = network.create_qubit();
	wire_id q2 = network.create_qubit();

	for (auto const& gate : one_qubit) {
		network.create_op(gate, q0);
	}
	for (auto const& gate : two_qubit) {
		network.create_op(gate, q0, q1);
	}
	for (auto const& gate : three_qubit) {
		network.create_op(gate, q0, q1, q2);
	}
	std::ostringstream os;
	write_qasm(network, os);

	std::string expected_qasm = "OPENQASM 2.0;\n"
	                            "include \"qelib1.inc\";\n"
	                            "qreg q[3];\n"
	                            "id q[0];\n"
	                            "h q[0];\n"
	                            "x q[0];\n"
	                            "y q[0];\n"
	                            "z q[0];\n"
	                            "s q[0];\n"
	                            "t q[0];\n"
	                            "sdg q[0];\n"
	                            "tdg q[0];\n"
	                            "cx q[0], q[1];\n"
	                            "cy q[0], q[1];\n"
	                            "cz q[0], q[1];\n"
	                            "swap q[0], q[1];\n"
	                            "ccx q[0], q[1], q[2];\n";
	CHECK(os.str() == expected_qasm);
}

TEMPLATE_PRODUCT_TEST_CASE("QASM Read/Write", "[qasm][template]", (netlist, op_dag),
                           (w3_op, wn32_op))
{
	std::string qasm = "OPENQASM 2.0;\n"
	                   "include \"qelib1.inc\";\n"
	                   "qreg q[3];\n"
	                   "id q[0];\n"
	                   "h q[0];\n"
	                   "x q[0];\n"
	                   "y q[0];\n"
	                   "z q[0];\n"
	                   "s q[0];\n"
	                   "t q[0];\n"
	                   "sdg q[0];\n"
	                   "tdg q[0];\n"
	                   "cx q[0], q[1];\n"
	                   "cy q[0], q[1];\n"
	                   "cz q[0], q[1];\n"
	                   "swap q[0], q[1];\n"
	                   "ccx q[0], q[1], q[2];\n";

	TestType network = read_qasm_from_buffer<TestType>(qasm);
	std::ostringstream os;
	write_qasm(network, os);
	CHECK(os.str() == qasm);
}
