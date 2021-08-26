/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/sat_swap_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Target/Device.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Synthesis of swapping networks using SAT", "[sat_swap_synth][synth]")
{
    Device device = Device::path(3u);
    Circuit expected;
    Qubit q0 = expected.create_qubit();
    Qubit q1 = expected.create_qubit();
    Qubit q2 = expected.create_qubit();
    SECTION("Swap (q0 , q2)")
    {
        expected.apply_operator(Op::Swap(), {q0, q2});
        std::vector<uint32_t> init_cfg = {0, 1, 2};
        std::vector<uint32_t> final_cfg = {2, 1, 0};
        Circuit synthesized = sat_swap_synth(device, init_cfg, final_cfg);
        CHECK(check_unitary(expected, synthesized));
    }
}

TEST_CASE("Synthesis of swapping networks using SAT2", "[sat_swap_synth][synth]")
{
    Device device = Device::path(5u);

    nlohmann::json config;
    config["sat_swap_synth"]["opt_goal"] = "depth";
    std::vector<uint32_t> init_cfg = {0, 1, 2, 3, 4};
    std::vector<uint32_t> final_cfg = {0, 2, 4, 3, 1};
    Circuit synthesized_swaps = sat_swap_synth(device, init_cfg, final_cfg);
    Circuit synthesized_depth =
      sat_swap_synth(device, init_cfg, final_cfg, config);
    CHECK(check_unitary(synthesized_swaps, synthesized_depth));
}
