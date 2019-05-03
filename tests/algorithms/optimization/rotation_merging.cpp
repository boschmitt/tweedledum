/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken, Fereshte Mozafari
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/optimization/rotation_merging.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;
template<typename Network>
bool check_optimized(Network const& orignal, Network const& optimized)
{
	pathsum_view orignal_sums(orignal);
	pathsum_view optimized_sums(optimized);

	uint32_t num_ok = 0;
	orignal_sums.foreach_output([&](auto const& node) {
		auto& sum = orignal_sums.get_pathsum(node);
		optimized_sums.foreach_output([&](auto const& node) {
			auto& sum2 = optimized_sums.get_pathsum(node);
			if (sum == sum2) {
				num_ok++;
			}
		});
	});
	return num_ok == orignal_sums.num_io();
}

TEMPLATE_PRODUCT_TEST_CASE("Rotation merging", "[rotation_merging][template]",
                           (gg_network, netlist), (mcmt_gate, io3_gate))
{
	TestType network;
	network.add_qubit("x1");
	network.add_qubit("x2");
	network.add_qubit("x3");
	network.add_qubit("x4");

	network.add_gate(gate::cx, "x3", "x4");

	network.add_gate(gate::t, "x1");
	network.add_gate(gate::t, "x4");

	network.add_gate(gate::cx, "x1", "x2");
	network.add_gate(gate::cx, "x3", "x4");

	network.add_gate(gate::cx, "x2", "x3");

	network.add_gate(gate::cx, "x2", "x1");
	network.add_gate(gate::cx, "x4", "x3");

	network.add_gate(gate::cx, "x2", "x3");

	network.add_gate(gate::cx, "x1", "x2");
	network.add_gate(gate::t_dagger, "x3");

	network.add_gate(gate::t, "x2");

	auto opt_network = rotation_merging(network);
	CHECK(check_optimized(network, opt_network));
}
