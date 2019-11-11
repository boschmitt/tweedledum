/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/constructors.hpp>
#include <kitty/static_truth_table.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <tweedledum/algorithms/generic/to_logic_network.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace mockturtle;
using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Conver simple quantum circuit to XAG", "[to_logic_network][template]",
                           (gg_network, netlist), (io3_gate, mcmt_gate))
{
	TestType quantum_ntk;
	auto q0 = quantum_ntk.add_qubit();
	auto q1 = quantum_ntk.add_qubit();
	auto q2 = quantum_ntk.add_qubit();

	quantum_ntk.add_gate(gate::mcx, std::vector<io_id>({q0, q1}), std::vector<io_id>({q2}));

	const auto logic_ntk = to_logic_network<xag_network>(quantum_ntk, std::vector<io_id>({q0, q1}),
	                                                                  std::vector<io_id>({q2}));

	kitty::dynamic_truth_table function(2u);
	kitty::create_from_binary_string(function, "1000");
	default_simulator<kitty::dynamic_truth_table> sim(function.num_vars());
	CHECK(simulate<kitty::dynamic_truth_table>(logic_ntk, sim)[0] == function);
}
