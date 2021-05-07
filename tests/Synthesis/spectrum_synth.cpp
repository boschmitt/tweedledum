/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/spectrum_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Standard.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>

TEST_CASE("Trivial cases for spectrum_synth", "[spectrum_synth][synth]")
{
    using namespace tweedledum;
    kitty::dynamic_truth_table tt(2);
    kitty::create_from_binary_string(tt, "1000");
    nlohmann::json config;

    Circuit synthesized = spectrum_synth(tt, config);
    CHECK(synthesized.num_qubits() == 3u);

    Circuit expected;
    Qubit q0 = expected.create_qubit();
    Qubit q1 = expected.create_qubit();
    Qubit q2 = expected.create_qubit();
    expected.apply_operator(Op::X(), {q0, q1, q2});
    CHECK(check_unitary(expected, synthesized));
}