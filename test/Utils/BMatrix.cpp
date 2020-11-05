/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Utils/Matrix.h"

#include <catch.hpp>

TEST_CASE("Initialization", "[BMatrix]")
{
    using namespace tweedledum;
    BMatrix matrix(3, 3);
    matrix << 1,0,0,
              0,1,0,
              0,0,1;
    CHECK(matrix == BMatrix::Identity(3,3));
}

TEST_CASE("Xor rows", "[BMatrix]")
{
    using namespace tweedledum;
    BMatrix matrix = BMatrix::Identity(3, 3);
    matrix.row(0) += matrix.row(1);

    BMatrix expected(3, 3);
    expected << 1,1,0,
                0,1,0,
                0,0,1;
    CHECK(matrix == expected);

    matrix.row(0) += matrix.row(1);
    CHECK(matrix == BMatrix::Identity(3,3));
}
