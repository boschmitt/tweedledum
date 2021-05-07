/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Placement.h"
// #include "../../IR/Circuit.h"
// #include "../../IR/Wire.h"
// #include "../../Target/Device.h"

#include <vector>

namespace tweedledum {

struct Mapping {
    Placement init_placement;
    Placement placement;

    Mapping(Placement const& init_placement)
        : init_placement(init_placement)
        , placement(init_placement)
    {}
};

} // namespace tweedledum
