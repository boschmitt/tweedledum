/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../IR/Instruction.h"
#include "../../IR/Wire.h"
#include "../../Target/Device.h"
#include "placer/JITPlacer.h"
#include "placer/LinePlacer.h"
#include "placer/RandomPlacer.h"
#include "placer/SabrePlacer.h"
#include "router/SabreRouter.h"
#include "MapState.h"

#include <string_view>

namespace tweedledum {

inline Circuit sabre_map(Circuit const& original, Device const& device)
{
    MapState state(original, device);
    RandomPlacer placer_rnd(state);
    placer_rnd.run();
    SabrePlacer placer(state);
    placer.run();
    SabreRouter router(state);
    router.run();
    return state.mapped;
}

} // namespace tweedledum
