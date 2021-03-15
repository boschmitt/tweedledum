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
    auto placement = line_place(device, original);
    JITRouter router(device, original, *placement);
    auto [circuit, mapping] = router.run();
    return circuit;
}

} // namespace tweedledum
