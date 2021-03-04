/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Circuit.h"

#include <catch.hpp>

TEST_CASE("Simple circuit functionality", "[circuit][ir]")
{
    using namespace tweedledum;
    Circuit circuit;

    WireRef qubit = circuit.create_qubit();
    WireRef cbit = circuit.create_cbit();
    CHECK(qubit.kind() == Wire::Kind::quantum);
    CHECK(cbit.kind() == Wire::Kind::classical);
}

