/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Cbit.h"

#include <catch.hpp>
#include <limits>

TEST_CASE("Classical bit (Cbit)", "[ir]")
{
    using namespace tweedledum;
    Cbit b0 = Cbit::invalid();
    CHECK(b0.uid() == (std::numeric_limits<uint32_t>::max() >> 1));

    b0 = Cbit(0, Cbit::Polarity::positive);
    Cbit not_b0(0, Cbit::Polarity::negative);
    SECTION("Test their polarity") {
        CHECK(b0.polarity() == Cbit::Polarity::positive);
        CHECK(not_b0.polarity() == Cbit::Polarity::negative);
        CHECK(b0.polarity() != not_b0.polarity());
    }
    SECTION("Test their uid") {
        CHECK(b0.uid() == not_b0.uid());
    }
    SECTION("Comparison") {
        CHECK(b0 != not_b0);
        CHECK(b0 == !not_b0);
        CHECK(!b0 == not_b0);
    }
    SECTION("Operators + and -") {
        Cbit cbit = Cbit(1337, Cbit::Polarity::positive);
        CHECK(cbit.uid() == 1337);
        CHECK(+cbit == cbit);
        CHECK(-cbit != +cbit);
        CHECK((-cbit).uid() == 1337);
        CHECK((-cbit).polarity() == Cbit::Polarity::negative);
    }
}
