/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"
#include "../../Target/Device.h"
#include "../../Target/Placement.h"
#include "Placer/RandomPlacer.h"
#include "RePlacer/SabreRePlacer.h"
#include "Router/SabreRouter.h"

#include <string_view>
#include <utility>

namespace tweedledum {

inline std::pair<Circuit, Mapping> sabre_map(
  Device const& device, Circuit const& original)
{
    auto placement = random_place(device, original);
    sabre_re_place(device, original, *placement);
    SabreRouter router(device, original, *placement);
    return router.run();
}

} // namespace tweedledum
