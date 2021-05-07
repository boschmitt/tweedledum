/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/cx_dihedral_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"
#include "tweedledum/Utils/LinPhasePoly.h"
#include "tweedledum/Utils/Numbers.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE(
  "Trivial cases for CX-Dihedral synthesis", "[cx_dihedral_synth][synth]")
{
    using namespace tweedledum;
    nlohmann::json config;
    LinPhasePoly phase_parities;
    BMatrix transform = BMatrix::Identity(3, 3);
    SECTION("Trivial case")
    {
        phase_parities.add_term(0b001, numbers::pi_div_4);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 1u);
    }
    SECTION("Still trivial, but with more rotations")
    {
        phase_parities.add_term(0b001, numbers::pi_div_4);
        phase_parities.add_term(0b010, numbers::pi_div_4);
        phase_parities.add_term(0b100, numbers::pi_div_4);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 3u);
    }
    SECTION("Will require one CX")
    {
        transform(0, 1) = 1;
        phase_parities.add_term(0b011, numbers::pi_div_4);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 2u);
    }
    SECTION("Will require two CX")
    {
        transform(0, 1) = 1;
        transform(1, 2) = 1;
        phase_parities.add_term(0b011, numbers::pi_div_4);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 3u);
    }
    SECTION("Will require two CX")
    {
        transform(0, 1) = 1;
        transform(1, 2) = 1;
        phase_parities.add_term(0b011, numbers::pi_div_4);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 3u);
    }
    SECTION("Will require two CX")
    {
        phase_parities.add_term(0b011, numbers::pi_div_4);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 3u);
    }
    SECTION("Will require more CX")
    {
        constexpr auto T = numbers::pi_div_4;
        constexpr auto T_dagger = -numbers::pi_div_4;
        phase_parities.add_term(0b001, T);
        phase_parities.add_term(0b010, T);
        phase_parities.add_term(0b100, T);
        phase_parities.add_term(0b011, T_dagger);
        phase_parities.add_term(0b101, T_dagger);
        phase_parities.add_term(0b110, T_dagger);
        phase_parities.add_term(0b111, T);
        auto circuit = cx_dihedral_synth(transform, phase_parities);
        CHECK(circuit.size() == 13u);
    }
}