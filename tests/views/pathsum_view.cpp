/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/views/pathsum_view.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Simple pathsum view", "[pathsum_view][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	TestType network;
	const auto a = network.add_qubit();
	const auto b = network.add_qubit();
	const auto c = network.add_qubit();
	const auto d = network.add_qubit();

	network.add_gate(gate::hadamard, a);
	network.add_gate(gate::cz, a, b);
	network.add_gate(gate::cz, b, c);
	network.add_gate(gate::cz, b, d);
	network.add_gate(gate::hadamard, d);

	pathsum_view sums(network);
}