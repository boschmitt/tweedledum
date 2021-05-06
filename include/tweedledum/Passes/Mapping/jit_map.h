/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Circuit.h"
#include "../../Target/Device.h"
#include "../Optimization/steiner_resynth.h"
#include "Placer/ApprxSatPlacer.h"
#include "Placer/LinePlacer.h"
#include "Router/JitRouter.h"

#include <string_view>

namespace tweedledum {

inline std::pair<Circuit, Mapping> jit_map(Device const& device,
    Circuit const& original)
{
    auto placement = apprx_sat_place(device, original);
    JitRouter router(device, original, *placement);
    auto [circuit, mapping] = router.run();
    if (original.size() < circuit.size()) {
        circuit = steiner_resynth(circuit, device);
    }
    return {circuit, mapping};
}

} // namespace tweedledum
