/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Qubit.h"

#include <catch.hpp>
#include <limits>

TEST_CASE("Quantum bit (Qubit)", "[ir]")
{
    using namespace tweedledum;
    Qubit q0 = Qubit::invalid();
    CHECK(q0.uid() == (std::numeric_limits<uint32_t>::max() >> 1));

    q0 = Qubit(0, Qubit::Polarity::positive);
    Qubit not_q0(0, Qubit::Polarity::negative);
    SECTION("Test their polarity") {
        CHECK(q0.polarity() == Qubit::Polarity::positive);
        CHECK(not_q0.polarity() == Qubit::Polarity::negative);
        CHECK(q0.polarity() != not_q0.polarity());
    }
    SECTION("Test their uid") {
        CHECK(q0.uid() == not_q0.uid());
    }
    SECTION("Comparison") {
        CHECK(q0 != not_q0);
        CHECK(q0 == !not_q0);
        CHECK(!q0 == not_q0);
    }
    SECTION("Operators + and -") {
        Qubit qubit = Qubit(1337, Qubit::Polarity::positive);
        CHECK(qubit.uid() == 1337);
        CHECK(+qubit == qubit);
        CHECK(-qubit != +qubit);
        CHECK((-qubit).uid() == 1337);
        CHECK((-qubit).polarity() == Qubit::Polarity::negative);
    }
}
