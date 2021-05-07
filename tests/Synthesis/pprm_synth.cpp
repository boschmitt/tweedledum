/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/pprm_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"

#include "../check_prime.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>

TEST_CASE("Synthesize a truth table using PPRM", "[pprm][synth]")
{
    using namespace tweedledum;
    SECTION("Constant-0")
    {
        kitty::dynamic_truth_table tt(1);
        Circuit circuit = pprm_synth(tt);
        REQUIRE(circuit.num_qubits() == 2);
        // number of operations = 1?
    }
    SECTION("Prime (3, .., 10)")
    {
        for (uint32_t i = 3; i < 10u; ++i) {
            kitty::dynamic_truth_table tt(i);
            kitty::create_prime(tt);
            Circuit circuit = pprm_synth(tt);
            REQUIRE(circuit.num_qubits() == i + 1);
            CHECK(check_prime(circuit));
        }
    }
}