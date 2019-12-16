/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../utils/device.hpp"
#include "token_swap/parameters.hpp"
#include "token_swap/a_star_swap.hpp"
#include "token_swap/sat_swap.hpp"

#include <utility>
#include <vector>

namespace tweedledum {

template<typename Network>
void swap_network(Network& network, device& topology, std::vector<uint32_t> const& final_mapping,
		  swap_network_params params = {})
{
	std::vector<uint32_t> init_mapping = network.phy_virtual_map();
	std::vector<std::pair<uint32_t, uint32_t>> swaps;
	switch (params.method) {
	case swap_network_params::methods::admissible:
	case swap_network_params::methods::non_admissible:
		swaps = detail::a_star_swap(topology, init_mapping, final_mapping, params);
		break;

	case swap_network_params::methods::sat:
		swaps = detail::sat_swap(topology, init_mapping, final_mapping, params);
		break;
	}
	for (auto [x, y] : swaps) {
		network.add_swap(x, y);
	}
	// assert(network.phy_virtual_map() == final_mapping);
	if (network.phy_virtual_map() != final_mapping) {
		auto map = network.phy_virtual_map();
		fmt::print("Mapped: {}\n", fmt::join(map.begin(), map.end(), ", "));
		fmt::print("Final:  {}\n", fmt::join(final_mapping.begin(), final_mapping.end(), ", "));
		std::abort();
	}
}

} // namespace tweedledum
