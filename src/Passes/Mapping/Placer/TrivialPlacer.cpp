/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Mapping/Placer/TrivialPlacer.h"

namespace tweedledum {

/*! \brief Yet to be written.
 */
std::optional<Placement> trivial_place(
  Device const& device, Circuit const& original)
{
    TrivialPlacer placer(device, original);
    return placer.run();
}

} // namespace tweedledum
