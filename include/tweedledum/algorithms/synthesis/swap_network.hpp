/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"
#include "../../networks/wire.hpp"
#include "../../utils/device.hpp"
#include "token_swap/a_star_swap.hpp"
#include "token_swap/parameters.hpp"
#include "token_swap/sat_swap.hpp"

#include <utility>
#include <vector>

namespace tweedledum {

void swap_network(mapped_dag& network, device& topology, std::vector<wire::id> const& phy_to_v,
		  swap_network_params params = {})
{
	std::vector<wire::id> init = network.phy_to_v();
	std::vector<std::pair<uint32_t, uint32_t>> swaps;

	std::vector<uint32_t> initial(init.begin(), init.end());
	std::vector<uint32_t> final(phy_to_v.begin(), phy_to_v.end());

	switch (params.method) {
	case swap_network_params::methods::admissible:
	case swap_network_params::methods::non_admissible:
		swaps = detail::a_star_swap(topology, initial, final, params);
		break;

	case swap_network_params::methods::sat:
		swaps = detail::sat_swap(topology, initial, final, params);
		break;
	}
	for (auto [x, y] : swaps) {
		network.create_op(gate_lib::swap, wire::make_qubit(x), wire::make_qubit(y));
		std::swap(init.at(x), init.at(y));
	}
	std::vector<wire::id> v_to_phy(topology.num_qubits(), wire::invalid_id);
	for (uint32_t i = 0u; i < phy_to_v.size(); ++i) {
		v_to_phy.at(phy_to_v.at(i)) = wire::make_qubit(i);
	}
	network.v_to_phy(v_to_phy);
}

} // namespace tweedledum
