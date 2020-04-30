/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"
#include "../../networks/wire.hpp"
#include "../../target/device.hpp"
#include "../utility/reverse.hpp"
#include "placement/hsat_placement.hpp"
#include "routing/sabre_router.hpp"

#include <vector>

namespace tweedledum {

/*! \brief SABRE-base mappper
 *
   \verbatim embed:rst

   Mapper based on the SABRE algorithm :cite:`SABRE19`.

   \endverbatim
 */
template<typename Network>
mapped_dag sabre_map(Network const& original, device const& device, sabre_config const& config = {})
{
	detail::sabre_router<Network> router(device, config);
	std::vector<wire::id> placement = detail::hsat_placement(original, device);
	mapped_dag mapped = router.route(original, placement);
	mapped = router.route(reverse(original), mapped.v_to_phy());
	return mapped;
}

} // namespace tweedledum
