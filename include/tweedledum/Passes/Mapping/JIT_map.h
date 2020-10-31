/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../Target/Device.h"
#include "Placer/LinePlacer.h"
#include "Router/JITRouter.h"
#include "MapState.h"

#include <string_view>

namespace tweedledum {

inline Circuit JIT_map(Circuit const& original, Device const& device)
{
    MapState state(original, device);
    LinePlacer placer_line(state);
    placer_line.run();
    JITRouter router(state);
    router.run();
    return state.mapped;
}

} // namespace tweedledum
