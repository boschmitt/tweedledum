/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/sabre_map.hpp"

#include "tweedledum/gates/gate_lib.hpp"
#include "tweedledum/gates/io3_gate.hpp"
#include "tweedledum/gates/mcmt_gate.hpp"
#include "tweedledum/networks/gg_network.hpp"
#include "tweedledum/utils/device.hpp"
#include "tweedledum/views/mapping_view.hpp"
#include "tweedledum/views/pathsum_view.hpp"

#include <catch.hpp>

using namespace tweedledum;

template<typename Network>
bool check_map(Network const& orignal, mapping_view<Network> const& mapped)
{
	pathsum_view orignal_sums(orignal, true);
	pathsum_view mapped_sums(mapped, mapped.init_virtual_phy_map(), true);

	uint32_t num_ok = 0;
	orignal_sums.foreach_output([&](auto const& node) {
		auto& sum = orignal_sums.get_pathsum(node);
		mapped_sums.foreach_output([&](auto const& node) {
			auto& sum2 = mapped_sums.get_pathsum(node);
			if (sum == sum2) {
				num_ok++;
			}
		});
	});

	// If something goes wrong, print pathsums!
	if (num_ok != orignal_sums.num_io()) {
		std::cout << "Pathsums original network: \n";
		orignal_sums.foreach_output([&](auto const& node) {
			auto& sum = orignal_sums.get_pathsum(node);
			for (auto e : sum) {
				std::cout << e << ' ';
			}
			std::cout << '\n';
		});

		std::cout << "Pathsums mapped network: \n";
		mapped_sums.foreach_output([&](auto const& node) {
			auto& sum = mapped_sums.get_pathsum(node);
			for (auto e : sum) {
				std::cout << e << ' ';
			}
			std::cout << '\n';
		});
	}
	return num_ok == orignal_sums.num_io();
}

// Some examples don't use the look-ahead because on them the perfomance while using it is really bad!
TEMPLATE_PRODUCT_TEST_CASE("Test for Sabre mapper", "[sabre_map][template]", (gg_network),
                           (mcmt_gate, io3_gate))
{
	TestType network;
	auto q0 = network.add_qubit();
	network.add_cbit();
	auto q1 = network.add_qubit();
	auto q2 = network.add_qubit();
	network.add_cbit();
	auto q3 = network.add_qubit();
	network.add_cbit();

	SECTION("Simple circuit")
	{
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);

		device arch = device::ring(network.num_qubits());
		auto mapped_ntk = sabre_map(network, arch);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend ZDD mapper paper example")
	{
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q3, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q3);

		device arch = device::ring(network.num_qubits());
		auto mapped_ntk = sabre_map(network, arch);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend ZDD mapper paper example #2")
	{
		network.add_qubit();
		auto q5 = network.add_qubit();

		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q5);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);

		device arch = device::ring(network.num_qubits());
		auto mapped_ntk = sabre_map(network, arch);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend ZDD mapper paper example #3")
	{
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q3, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q3, q2);
		network.add_gate(gate::cx, q3, q1);
		network.add_gate(gate::cx, q3, q0);

		device arch = device::ring(network.num_qubits());
		auto mapped_ntk = sabre_map(network, arch);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend ZDD mapper paper example #3.5")
	{
		auto q4 = network.add_qubit();
		auto q5 = network.add_qubit();
		auto q6 = network.add_qubit();
		auto q7 = network.add_qubit();
		auto q8 = network.add_qubit();
		auto q9 = network.add_qubit();

		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q5);
		network.add_gate(gate::cx, q9, q8);
		network.add_gate(gate::cx, q1, q5);
		network.add_gate(gate::cx, q4, q3);
		network.add_gate(gate::cx, q8, q7);
		network.add_gate(gate::cx, q6, q8);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q2, q5);
		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);

		device arch = device::ring(network.num_qubits());
		sabre_map_params ps;
		auto mapped_ntk = sabre_map(network, arch, ps);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend ZDD mapper paper #4")
	{
		auto q4 = network.add_qubit();
		auto q5 = network.add_qubit();
		auto q6 = network.add_qubit();
		auto q7 = network.add_qubit();

		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q1, q3);
		network.add_gate(gate::cx, q4, q5);
		network.add_gate(gate::cx, q5, q6);
		network.add_gate(gate::cx, q5, q7);

		device arch = device::ring(network.num_qubits());
		sabre_map_params ps;
		auto mapped_ntk = sabre_map(network, arch, ps);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Test for ZDD mapper")
	{
		auto q4 = network.add_qubit();
		auto q5 = network.add_qubit();

		network.add_gate(gate::cx, q0, q2);
		network.add_gate(gate::cx, q2, q1);
		network.add_gate(gate::cx, q0, q4);
		network.add_gate(gate::cx, q3, q0);
		network.add_gate(gate::cx, q0, q5);

		device arch = device::ring(network.num_qubits());
		auto mapped_ntk = sabre_map(network, arch);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Test two consecutive swaps mapper")
	{
		auto q4 = network.add_qubit();

		network.add_gate(gate::cx, q0, q1);
		network.add_gate(gate::cx, q1, q2);
		network.add_gate(gate::cx, q2, q3);
		network.add_gate(gate::cx, q3, q4);
		network.add_gate(gate::cx, q0, q4);

		device arch = device::path(network.num_qubits());
		sabre_map_params ps;
		auto mapped_ntk = sabre_map(network, arch, ps);
		CHECK(check_map(network, mapped_ntk));
	}
}
