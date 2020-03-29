/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include "tweedledum/algorithms/synthesis/swap_network.hpp"

#include "tweedledum/gates/gate.hpp"
#include "tweedledum/networks/mapped_dag.hpp"
#include "tweedledum/networks/wire_id.hpp"
#include "tweedledum/utils/device.hpp"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Synthesis of swapping networks using A*", "[swap_network][synth]")
{
	device arch = device::path(3u);
	mapped_dag swap_mapped(arch);

	std::vector<wire_id> final = {wire_id(2, true), wire_id(1, true), wire_id(0, true)};
	swap_network(swap_mapped, arch, final);
	CHECK(swap_mapped.phy_to_v() == final);
}

TEST_CASE("Synthesis of swapping networks using SAT", "[swap_network][synth]")
{
	device arch = device::path(3u);
	mapped_dag swap_mapped(arch);

	std::vector<wire_id> final = {wire_id(2, true), wire_id(1, true), wire_id(0, true)};

	swap_network_params params;
	params.method = swap_network_params::methods::sat;
	swap_network(swap_mapped, arch, final, params);
	CHECK(swap_mapped.phy_to_v() == final);
}
