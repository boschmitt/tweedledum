/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Target/Device.h"

#include <catch.hpp>

using Path = std::vector<uint32_t>;

TEST_CASE("Test distances", "[Device]")
{
    using namespace tweedledum;
    SECTION("Path topology")
    {
        Device device = Device::path(4);
        CHECK(device.distance(0, 1) == 1);
        CHECK(device.distance(0, 2) == 2);
        CHECK(device.distance(0, 3) == 3);
    }
}

TEST_CASE("Test shortest paths", "[device]")
{
    using namespace tweedledum;
    SECTION("Path topology")
    {
        Device device = Device::path(4);
        for (uint32_t i = 0u; i < device.num_qubits(); ++i) {
            CHECK(device.shortest_path(i, i) == Path({}));
        }
        CHECK(device.shortest_path(0, 1) == Path({0, 1}));
        CHECK(device.shortest_path(0, 2) == Path({0, 1, 2}));
        CHECK(device.shortest_path(0, 3) == Path({0, 1, 2, 3}));

        CHECK(device.shortest_path(1, 0) == Path({1, 0}));
        CHECK(device.shortest_path(2, 0) == Path({2, 1, 0}));
        CHECK(device.shortest_path(3, 0) == Path({3, 2, 1, 0}));
    }
}
