/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"
#include "../../networks/wire.hpp"
#include "../../utils/device.hpp"
#include "../transformation/reverse.hpp"
#include "placement/line_placement.hpp"
#include "routing/jit_router.hpp"

#include <vector>

namespace tweedledum {

/*! \brief Yet to be written.
 */
template<typename Network>
mapped_dag jit_map(Network const& original, device const& device, jit_config const& config = {})
{
	detail::jit_router<Network> router(device, config);
	auto reversed = reverse(original);
	std::vector<wire::id> placement = detail::line_placement(reversed, device);
	mapped_dag mapped = router.route(original, placement, false);
	mapped = router.route(reversed, mapped.v_to_phy());
	return mapped;
}

} // namespace tweedledum
