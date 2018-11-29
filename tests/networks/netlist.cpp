/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <string>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

TEST_CASE("Netlist simple constructor", "[netlist]")
{
	using namespace tweedledum;
	netlist<mcst_gate> network;

	CHECK(network.size() == 0);

	network.add_qubit("q0");
	CHECK(network.size() == 2);
	CHECK(network.num_qubits() == 1);

	network.add_qubit();
	CHECK(network.size() == 4);
	CHECK(network.num_qubits() == 2);
}

TEST_CASE("Netlist const iterators", "[netlist]")
{
	using namespace tweedledum;
	netlist<mcst_gate> network;
	network.add_qubit("q0");
	network.add_qubit("q1");

	network.foreach_cqubit([](auto qid, auto const& qlabel) {
		if (qid == 0) {
			CHECK(qlabel == std::string("q0"));
		}
		if (qid == 1) {
			CHECK(qlabel == std::string("q1"));
		}
	});
}
