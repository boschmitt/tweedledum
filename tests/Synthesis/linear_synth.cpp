/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/linear_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Utils/Matrix.h"
#include "tweedledum/Utils/Visualization/string_utf8.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Trivial cases for linear_synth", "[linear_synth][synth]")
{
    using namespace tweedledum;
    BMatrix linear_trans(3, 3);
    linear_trans << 1,1,1,
                    0,1,1,
                    0,0,1;

    nlohmann::json config;
    Circuit synthesized = linear_synth(linear_trans, config);

    Circuit expected;
    Qubit q0 = expected.create_qubit();
    Qubit q1 = expected.create_qubit();
    Qubit q2 = expected.create_qubit();
    expected.apply_operator(Op::X(), {q2, q1});
    expected.apply_operator(Op::X(), {q1, q0});
    CHECK(check_unitary(expected, synthesized));
}
