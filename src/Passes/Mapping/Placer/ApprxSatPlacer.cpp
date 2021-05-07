/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Placer/ApprxSatPlacer.h"

namespace tweedledum {

/*! \brief Yet to be written.
 */
std::optional<Placement> apprx_sat_place(
  Device const& device, Circuit const& original, nlohmann::json const& config)
{
    bool use_weight = true;
    bill::solver solver;
    ApprxSatPlacer placer(device, original, solver, use_weight);
    return placer.run();
}

} // namespace tweedledum
