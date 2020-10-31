/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Synthesis/all_linear_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Utils/LinearPP.h"

#include <catch.hpp>

TEST_CASE("Trivial cases for all linear synthe", "[all-linear][synth]")
{
    using namespace tweedledum;
    LinearPP parities;
    Circuit circuit = all_linear_synth(1, parities);
    CHECK(circuit.size() == 0);
}