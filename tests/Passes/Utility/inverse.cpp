/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Utility/inverse.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"

#include "../check_inverse.h"
#include "../test_circuits.h"

#include <catch.hpp>

using namespace tweedledum;

TEST_CASE("Trivial test cases for inverting (take adjoint of) circuits.",
  "[inverse][utility]")
{
    using namespace tweedledum;
    SECTION("Toffoli operator")
    {
        Circuit circuit = toffoli();
        std::optional<Circuit> adjoint = inverse(circuit);
        CHECK(adjoint);
        CHECK(check_inverse(circuit, *adjoint));
    }
    SECTION("Graph coloring init")
    {
        Circuit circuit = graph_coloring_init();
        std::optional<Circuit> adjoint = inverse(circuit);
        CHECK(adjoint);
        CHECK(check_inverse(circuit, *adjoint));
    }
}
