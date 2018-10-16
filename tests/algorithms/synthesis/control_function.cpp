/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <cstdint>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <tweedledum/algorithms/synthesis/control_function.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

using namespace tweedledum;

TEST_CASE("Synthesize MAJ-3 using PPRM-based synthesis", "[control_function]")
{
	kitty::dynamic_truth_table f(3);
	kitty::create_majority(f);
	const auto circ = control_function_synthesis<netlist<mcmt_gate>>(f);

	CHECK(circ.num_gates() == 3u);
	CHECK(circ.num_qubits() == 4u);
}

TEST_CASE("Synthesize 2-input OR using PPRM-based synthesis", "[control_function]")
{
	kitty::dynamic_truth_table f(2);
	kitty::create_from_binary_string(f, "1110");
	const auto circ = control_function_synthesis<netlist<mcmt_gate>>(f);

	CHECK(circ.num_gates() == 3u);
	CHECK(circ.num_qubits() == 3u);
}

TEST_CASE("Synthesize 2-input OR using PKRM-based synthesis", "[control_function]")
{
	kitty::dynamic_truth_table f(2);
	kitty::create_from_binary_string(f, "1110");
	const auto circ = control_function_synthesis<netlist<mcmt_gate>, stg_from_pkrm>(f);

	CHECK(circ.num_gates() == 4u);
	CHECK(circ.num_qubits() == 3u);
}

TEST_CASE("Synthesize 3-input Toffoli using spectral-based synthesis", "[control_function]")
{
	kitty::dynamic_truth_table f(3);
	kitty::create_from_binary_string(f, "10000000");
	const auto circ = control_function_synthesis<netlist<mcst_gate>, stg_from_spectrum>(f);

	CHECK(circ.num_gates() == 30u);
	CHECK(circ.num_qubits() == 4u);
}
