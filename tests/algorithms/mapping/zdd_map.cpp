/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Kate Smith
*------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/mapping/zdd_map.hpp"

#include "tweedledum/gates/gate_set.hpp"
#include "tweedledum/gates/mcst_gate.hpp"
#include "tweedledum/gates/mcmt_gate.hpp"
#include "tweedledum/io/write_unicode.hpp"
#include "tweedledum/networks/gg_network.hpp"
#include "tweedledum/utils/device.hpp"
#include "tweedledum/views/mapping_view.hpp"

#include <catch.hpp>

using namespace tweedledum;

template<typename Network>
bool check_map(Network const& orignal, mapping_view<Network> const& mapped)
{
	pathsum_view orignal_sums(orignal, true);
	pathsum_view mapped_sums(mapped, mapped.init_virtual_phy_map(), true);

	uint32_t num_ok = 0;
	orignal_sums.foreach_coutput([&](auto const& node) {
		auto& sum = orignal_sums.get_pathsum(node);
		mapped_sums.foreach_coutput([&](auto const& node) {
			auto& sum2 = mapped_sums.get_pathsum(node);
			if (sum == sum2) {
				num_ok++;
			}
		});
	});

	// If something goes wrong, print pathsums!
	if (num_ok != orignal_sums.num_qubits()) {
		std::cout << "Pathsums original network: \n";
		orignal_sums.foreach_coutput([&](auto const& node) {
			auto& sum = orignal_sums.get_pathsum(node);
			for (auto e : sum) {
				std::cout << e << ' ';
			}
			std::cout << '\n';
		});

		std::cout << "Pathsums mapped network: \n";
		mapped_sums.foreach_coutput([&](auto const& node) {
			auto& sum = mapped_sums.get_pathsum(node);
			for (auto e : sum) {
				std::cout << e << ' ';
			}
			std::cout << '\n';
		});
	}
	return num_ok == orignal_sums.num_qubits();
}

TEMPLATE_PRODUCT_TEST_CASE("Test for ZDD mapper", "[zdd_map][template]", (gg_network),
                           (mcmt_gate, mcst_gate))
{
	SECTION("Test reading in quil")
	{
		TestType network;

		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend paper example for ZDD mapper")
	{
		TestType network;

		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);
		network.add_gate(gate::cx, 2, 3);
		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);
		network.add_gate(gate::cx, 2, 3);
		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 3, 2);
		network.add_gate(gate::cx, 1, 3);
		network.add_gate(gate::cx, 2, 3);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend paper example #2 for ZDD mapper")
	{
		TestType network;

		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);
		network.add_gate(gate::cx, 2, 5);
		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend paper example #3 for ZDD mapper")
	{
		TestType network;
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		network.add_gate(gate::cx, 2, 3);

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		network.add_gate(gate::cx, 2, 3);

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 3, 2);
		network.add_gate(gate::cx, 1, 3);

		network.add_gate(gate::cx, 2, 3);

		network.add_gate(gate::cx, 3, 2);
		network.add_gate(gate::cx, 3, 1);
		network.add_gate(gate::cx, 3, 0);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Extend paper example #3.5 for ZDD mapper")
	{
		TestType network;
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		network.add_gate(gate::cx, 2, 5);

		network.add_gate(gate::cx, 9, 8);
		network.add_gate(gate::cx, 1, 5);
		network.add_gate(gate::cx, 4, 3);

		network.add_gate(gate::cx, 8, 7);
		network.add_gate(gate::cx, 6, 8);
		network.add_gate(gate::cx, 1, 3);

		network.add_gate(gate::cx, 2, 5);

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Paper example #4 for ZDD mapper")
	{
		TestType network;
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 1, 3);

		network.add_gate(gate::cx, 4, 5);
		network.add_gate(gate::cx, 5, 6);
		network.add_gate(gate::cx, 5, 7);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Test for ZDD mapper")
	{
		TestType network;
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();

		network.add_gate(gate::cx, 0, 2);
		network.add_gate(gate::cx, 2, 1);
		network.add_gate(gate::cx, 0, 4);
		network.add_gate(gate::cx, 3, 0);
		network.add_gate(gate::cx, 0, 5);

		device arch = device::ring(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
	}

	SECTION("Test two consecutive swaps ZDD mapper")
	{
		TestType network;
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();
		network.add_qubit();

		network.add_gate(gate::cx, 0, 1);
		network.add_gate(gate::cx, 1, 2);
		network.add_gate(gate::cx, 2, 3);
		network.add_gate(gate::cx, 3, 4);
		network.add_gate(gate::cx, 0, 4);

		device arch = device::path(network.num_qubits());
		zdd_map_params ps;
		ps.verbose = false;
		zdd_map_stats st;
		auto mapped_ntk = zdd_map(network, arch, ps, &st);
		CHECK(check_map(network, mapped_ntk));
		CHECK(mapped_ntk.is_partial());
	}
}
