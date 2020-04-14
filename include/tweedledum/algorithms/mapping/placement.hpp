/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/wire.hpp"
#include "../../utils/device.hpp"
#include "placement/random_placement.hpp"
#include "placement/line_placement.hpp"
#include "placement/hsat_placement.hpp"

namespace tweedledum {

/*! \brief Parameters for `placment`. */
struct placement_config {
	enum class methods {
		/*! \brief (**default**) */
		greedy,
		/*! \brief */
		greedy_sat,
		/*! \brief */
		line,
		/*! \brief */
		random,
	};
	methods method = methods::greedy_sat;
	uint32_t random_seed = 17u;
};

/*! \brief Yet to be written.
 */
template<typename Network>
std::vector<wire::id> placement(Network const& network, device const& device,
                                placement_config params = {})
{
	using methods = placement_config::methods;

	switch (params.method) {
	case methods::greedy:
		return {};

	case methods::greedy_sat:
		return detail::hsat_placement(network, device);

	case methods::line:
		return detail::line_placement(network, device);

	case methods::random:
		return detail::random_placement(device, params.random_seed);
	}
}

} // namespace tweedledum
