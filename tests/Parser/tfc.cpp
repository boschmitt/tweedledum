/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Parser/tfc.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Operators/Standard/X.h"

#include "../check_unitary.h"

#include <catch.hpp>

TEST_CASE("TFC Parsing", "[tfc][parser]")
{
    using namespace tweedledum;
    SECTION("Empty buffer")
    {
        std::string tfc = "";
        Circuit parsed = tfc::parse_source_buffer(tfc);
        CHECK(parsed.size() == 0u);
    }
    SECTION("Circuit without instructions")
    {
        std::string tfc = "# A comment.\n"
                          ".v a,b c1, d5\n";
        Circuit parsed = tfc::parse_source_buffer(tfc);
        CHECK(parsed.size() == 0u);
        CHECK(parsed.num_qubits() == 4u);
        CHECK(parsed.num_cbits() == 0u);
    }
    SECTION("Circuit without instructions")
    {
        std::string tfc = "# A comment.\n"
                          ".v a,b c1, d5\n"
                          "BEGIN\n"
                          "t1 a\n"
                          "\n"
                          " f2 a, b\n"
                          "# Another comment\n"
                          "t4 a,b c1, d5\n"
                          "f3 a,b,d5\n"
                          "END\n";
        Circuit parsed = tfc::parse_source_buffer(tfc);
        CHECK(parsed.size() == 4u);
        CHECK(parsed.num_qubits() == 4u);
        CHECK(parsed.num_cbits() == 0u);
        Circuit expected;
        Qubit a = expected.create_qubit();
        Qubit b = expected.create_qubit();
        Qubit c1 = expected.create_qubit();
        Qubit d5 = expected.create_qubit();
        expected.apply_operator(Op::X(), {a});
        expected.apply_operator(Op::Swap(), {a, b});
        expected.apply_operator(Op::X(), {a, b, c1, d5});
        expected.apply_operator(Op::Swap(), {a, b, d5});
        CHECK(check_unitary(expected, parsed));
    }
}
