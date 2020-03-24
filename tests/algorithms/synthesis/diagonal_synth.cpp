/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/diagonal_synth.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/utils/angle.hpp"

#include <catch.hpp>
#include <sstream>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Synthesize diagonal unitaries", "[diagonal_synth][template]",
                           (netlist, op_dag), (w3_op, wn32_op))
{
	SECTION("One-qubit"){
		std::vector<angle> angles = {sym_angle::pi};
		TestType network = diagonal_synth<TestType>(angles);
		// CHECK(network.num_qubits() == 1u);
		// CHECK(network.num_operations() == 1u);
	}
	SECTION("Two-qubit controlled r1(pi)") {
		std::vector<angle> angles = {sym_angle::zero, sym_angle::zero, sym_angle::pi};
		TestType network = diagonal_synth<TestType>(angles);
		// CHECK(network.num_qubits() == 2u);
		// CHECK(network.num_operations() == 5u);
	}
	SECTION("Two-qubit controlled rz(pi/2)") {
		std::vector<angle> angles = {sym_angle::zero, -sym_angle::pi_half, sym_angle::pi_half};
		TestType network = diagonal_synth<TestType>(angles);
		// CHECK(network.num_qubits() == 2u);
		// CHECK(network.num_operations() == 2u);
	}
	SECTION("Three-qubit Toffoli") {
		std::vector<angle> angles(6, sym_angle::zero);
		angles.push_back(sym_angle::pi_half);
		TestType network = diagonal_synth<TestType>(angles);
		// CHECK(network.num_qubits() == 3u);
	}
	SECTION("Three-qubit Toffoli") {
		std::vector<angle> angles(5, sym_angle::zero);
		angles.push_back(-sym_angle::pi);
		angles.push_back(sym_angle::pi);
		TestType network = diagonal_synth<TestType>(angles);
		// CHECK(network.num_qubits() == 3u);
		// std::ostringstream os;
		// write_qasm(network, os);
		// std::cout << os.str();
	}


}
