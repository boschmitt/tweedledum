/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <string>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>

TEST_CASE("Netlist simple constructors", "[netlist]")
{
	using namespace tweedledum;

	SECTION("No gates") {
		netlist<mcmt_gate> network;
		CHECK(network.size() == 0);

		network.add_qubit("q0");
		network.add_qubit();
		CHECK(network.size() == 4);
		CHECK(network.num_qubits() == 2);
	}
	SECTION("One gate") {
		netlist<mcmt_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::cx, q0, q1);
		CHECK(network.size() == 6);
		CHECK(network.num_qubits() == 2);
		write_unicode(network);
	}
	SECTION("One gate, negative control") {
		netlist<mcmt_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		network.add_gate(gate::hadamard, q0);
		network.add_gate(gate::cx, !q0, q1);
		CHECK(network.size() == 6);
		CHECK(network.num_qubits() == 2);
		write_unicode(network);
	}
	SECTION("One gate, negative control") {
		netlist<mcmt_gate> network;
		auto q0 = network.add_qubit("q0");
		auto q1 = network.add_qubit();
		auto q2 = network.add_qubit();
		network.add_gate(gate::mcx, std::vector({!q0, q1}), std::vector({q2}));
		CHECK(network.size() == 7);
		CHECK(network.num_qubits() == 3);
		write_unicode(network);
	}

}

TEST_CASE("Netlist const iterators", "[netlist]")
{
	using namespace tweedledum;
	netlist<mcmt_gate> network;
	network.add_qubit("q0");
	network.add_qubit("q1");

	network.foreach_cqubit([](auto qid, auto const& qlabel) {
		if (qid == qubit_id(0)) {
			CHECK(qlabel == std::string("q0"));
		}
		if (qid == qubit_id(1)) {
			CHECK(qlabel == std::string("q1"));
		}
	});

	network.add_gate(gate::hadamard, "q0");
	network.add_gate(gate::cx, "q0", "q1");
	network.add_gate(gate::hadamard, "q0");
	network.add_gate(gate::cx, "q0", "q1");
	network.add_gate(gate::cx, "q1", "q0");
	write_unicode(network);
}
