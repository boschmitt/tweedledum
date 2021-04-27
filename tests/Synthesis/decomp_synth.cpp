/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/decomp_synth.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"

#include "../check_permutation.h"

#include <catch.hpp>
#include <kitty/kitty.hpp>

// DBS = Decomposition-based synthesis
TEST_CASE("Synthesize a permutation using DBS", "[decomp_synth][synth]")
{
    using namespace tweedledum;
    SECTION("Prime 3") {
        std::vector<uint32_t> prime = {0, 2, 3, 5, 7, 1, 4, 6};
        Circuit circuit = decomp_synth(prime);
        CHECK(check_permutation(circuit, prime));
    }
    SECTION("Prime 4") {
        std::vector<uint32_t> prime
            = {0, 2, 3, 5, 7, 11, 13, 1, 4, 6, 8, 9, 10, 12, 14, 15};
        Circuit circuit = decomp_synth(prime);
        CHECK(check_permutation(circuit, prime));
    }
    SECTION("Prime 5") {
        std::vector<uint32_t> prime = {0, 2, 3, 5, 7, 11, 13, 17, 19,
            23, 29, 31, 1, 4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21,
            22, 24, 25, 26, 27, 28, 30};
        Circuit circuit = decomp_synth(prime);
        CHECK(check_permutation(circuit, prime));
    }
    SECTION("Prime 6") {
        std::vector<uint32_t> prime = {0, 2, 3, 5, 7, 11, 13, 17, 19,
            23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 1, 4, 6, 8, 9, 10,
            12, 14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 32,
            33, 34, 35, 36, 38, 39, 40, 42, 44, 45, 46, 48, 49, 50, 51,
            52, 54, 55, 56, 57, 58, 60, 62, 63};
        Circuit circuit = decomp_synth(prime);
        CHECK(check_permutation(circuit, prime));
    }
    SECTION("Prime 8") {
        std::vector<uint32_t> prime = {0, 2, 3, 5, 7, 11, 13, 17, 19,
            23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83,
            89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
            151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
            223, 227, 229, 233, 239, 241, 251, 1, 4, 6, 8, 9, 10, 12,
            14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 32, 33,
            34, 35, 36, 38, 39, 40, 42, 44, 45, 46, 48, 49, 50, 51, 52,
            54, 55, 56, 57, 58, 60, 62, 63, 64, 65, 66, 68, 69, 70, 72,
            74, 75, 76, 77, 78, 80, 81, 82, 84, 85, 86, 87, 88, 90, 91,
            92, 93, 94, 95, 96, 98, 99, 100, 102, 104, 105, 106, 108,
            110, 111, 112, 114, 115, 116, 117, 118, 119, 120, 121, 122,
            123, 124, 125, 126, 128, 129, 130, 132, 133, 134, 135, 136,
            138, 140, 141, 142, 143, 144, 145, 146, 147, 148, 150, 152,
            153, 154, 155, 156, 158, 159, 160, 161, 162, 164, 165, 166,
            168, 169, 170, 171, 172, 174, 175, 176, 177, 178, 180, 182,
            183, 184, 185, 186, 187, 188, 189, 190, 192, 194, 195, 196,
            198, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
            212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 224,
            225, 226, 228, 230, 231, 232, 234, 235, 236, 237, 238, 240,
            242, 243, 244, 245, 246, 247, 248, 249, 250, 252, 253, 254,
            255};
        Circuit circuit = decomp_synth(prime);
        CHECK(check_permutation(circuit, prime));
    }
}