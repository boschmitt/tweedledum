/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/mapping/device.hpp>
#include <tweedledum/algorithms/mapping/greedymap.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;

TEST_CASE("Simple example for Greedy mapper", "[greedymap]")
{
	using namespace tweedledum;
	netlist<mcst_gate> network;
	const auto a = network.add_qubit();
	const auto b = network.add_qubit();
	const auto c = network.add_qubit();
	const auto d = network.add_qubit();

  network.add_gate(gate::hadamard, a);
	network.add_gate(gate::cz, a, b);
	network.add_gate(gate::cz, b, c);
	network.add_gate(gate::cz, b, d);
	network.add_gate(gate::hadamard, d);

	write_unicode(network);

	const auto mapped = greedy_map(network, device_t::ring(network.num_qubits()));
	if (mapped) {
		std::cout << "\n";
		write_unicode(*mapped);
	}
}

