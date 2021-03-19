/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/gray_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"
#include "tweedledum/Utils/LinPhasePoly.h"
#include "tweedledum/Utils/Numbers.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Trivial cases for gray_synth", "[gray_synth][synth]")
{
    using namespace tweedledum;
    nlohmann::json config;
    SECTION("Check simple example from Amy paper") {
        LinPhasePoly phase_parities;
        phase_parities.add_term(0b0110, numbers::pi_div_4);
        phase_parities.add_term(0b0001, numbers::pi_div_4);
        phase_parities.add_term(0b1001, numbers::pi_div_4);
        phase_parities.add_term(0b0111, numbers::pi_div_4);
        phase_parities.add_term(0b1011, numbers::pi_div_4);
        phase_parities.add_term(0b0011, numbers::pi_div_4);
        Circuit synthesized = gray_synth(4, phase_parities, config);
        BMatrix transform = BMatrix::Identity(4, 4);
        synthesized.foreach_instruction([&transform](Instruction const& inst) {
            if (inst.num_qubits() == 1) {
                return;
            }
            transform.row(inst.target()) += transform.row(inst.control());
        });
        CHECK(transform.isIdentity());
        CHECK(synthesized.size() == 15u);

        // The example in the paper is wrong! :( 
        // (it took me a while to figure it out)
        Circuit expected;
        Qubit q0 = expected.create_qubit();
        Qubit q1 = expected.create_qubit();
        Qubit q2 = expected.create_qubit();
        Qubit q3 = expected.create_qubit();
        expected.apply_operator(Op::T(), {q0});
        expected.apply_operator(Op::X(), {q2, q1});
        expected.apply_operator(Op::T(), {q1});
        expected.apply_operator(Op::X(), {q3, q0});
        expected.apply_operator(Op::T(), {q0});
        expected.apply_operator(Op::X(), {q1, q0});
        expected.apply_operator(Op::X(), {q3, q0});
        expected.apply_operator(Op::T(), {q0});
        expected.apply_operator(Op::X(), {q2, q0});
        expected.apply_operator(Op::T(), {q0});
        expected.apply_operator(Op::X(), {q3, q0});
        expected.apply_operator(Op::T(), {q0});
        expected.apply_operator(Op::X(), {q2, q1});
        expected.apply_operator(Op::X(), {q1, q0});
        expected.apply_operator(Op::X(), {q3, q0});
        CHECK(check_unitary(expected, synthesized));
    }
}