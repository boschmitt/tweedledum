/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Target/Device.h"

#include <catch.hpp>

TEST_CASE("Test shortest path algorithm", "[Device]")
{
    using namespace tweedledum;
    SECTION("Path topology") {
        Device path_4 = Device::path(4);
        CHECK(path_4.distance(0, 1) == 1);
        CHECK(path_4.distance(0, 2) == 2);
        CHECK(path_4.distance(0, 3) == 3);
    }
    // Device grid_4x4 = Device::grid(4, 4);
}
