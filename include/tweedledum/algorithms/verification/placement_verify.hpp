/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/wire_id.hpp"
#include "../../utils/device.hpp"

#include <vector>

namespace tweedledum {

/* \brief Check if a placement is consistent
 *
 */
inline bool placement_verify(device const& device, std::vector<wire_id> const& placement)
{
	if (device.num_qubits() != placement.size()) {
		return false;
	}
	std::vector<uint32_t> phy_occurences(device.num_qubits(), 0u);
	for (uint32_t i = 0u; i < placement.size(); ++i) {
		if (placement.at(i) == wire::invalid) {
			continue;
		}
		if (++phy_occurences.at(placement.at(i)) >= 2u) {
			return false;
		}
	}
	return true;
}

} // namespace tweedledum
