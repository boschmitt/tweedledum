/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/wire.hpp"
#include "../../utils/device.hpp"

#include <vector>

namespace tweedledum {

/*! \brief Verify if a placement is consistent.
 *
 * The verification of a placement only requires checking whether the placement is consistent, i.e.,
 * if the number of virtual qubits (v) is the samse as the number of physical qubits (phy) and
 * making sure that each physical qubit has at most one virtual qubit.
 * 
 * \param[in] device information about the physical device.
 * \param[in] placement the placement is a mapping v -> phy.
 * \returns true if the placement is consistent.
 */
inline bool placement_verify(device const& device, std::vector<wire::id> const& placement)
{
	if (device.num_qubits() != placement.size()) {
		return false;
	}
	std::vector<uint32_t> phy_occurences(device.num_qubits(), 0u);
	for (uint32_t i = 0u; i < placement.size(); ++i) {
		if (placement.at(i) == wire::invalid_id) {
			continue;
		}
		if (++phy_occurences.at(placement.at(i)) >= 2u) {
			return false;
		}
	}
	return true;
}

} // namespace tweedledum
