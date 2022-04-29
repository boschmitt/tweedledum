/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Placer/SatPlacer.h"

namespace tweedledum {

/*! \brief Yet to be written.
 */
std::optional<Placement> sat_place(
  Device const& device, Circuit const& original)
{
    bill::solver solver;
    SatPlacer placer(device, original, solver);
    return placer.run();
}

} // namespace tweedledum
