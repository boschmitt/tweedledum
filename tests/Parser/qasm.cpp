/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Parser/qasm.h"

#include <catch.hpp>

TEST_CASE("QASM Parsing", "[qasm][parser]")
{
    using namespace tweedledum;
    SECTION("Empty buffer")
    {
        std::string qasm = "";
        Circuit circuit = qasm::parse_source_buffer(qasm);
        CHECK(circuit.size() == 0u);
    }
    SECTION("Empty circuit")
    {
        std::string qasm = "OPENQASM 2.0;\n"
                           "include \"qelib1.inc\";\n";
        Circuit circuit = qasm::parse_source_buffer(qasm);
        CHECK(circuit.size() == 0u);
    }
    SECTION("Circuit without instructions")
    {
        std::string qasm = "OPENQASM 2.0;\n"
                           "include \"qelib1.inc\";\n"
                           "qreg q[32];";
        Circuit circuit = qasm::parse_source_buffer(qasm);
        CHECK(circuit.size() == 0u);
        CHECK(circuit.num_qubits() == 32u);
        CHECK(circuit.num_cbits() == 0u);
    }
    SECTION("Toffoli")
    {
        std::string qasm = "OPENQASM 2.0;\n"
                           "include \"qelib1.inc\";\n"
                           "qreg a[3];"
                           "x a[0];"
                           "x a[1];"
                           "h a[2];"
                           "cx a[1],a[2];"
                           "tdg a[2];"
                           "cx a[0],a[2];"
                           "t a[2];"
                           "cx a[1],a[2];"
                           "tdg a[2];"
                           "cx a[0],a[2];"
                           "tdg a[1];"
                           "t a[2];"
                           "cx a[0],a[1];"
                           "h a[2];"
                           "tdg a[1];"
                           "cx a[0],a[1];"
                           "t a[0];"
                           "s a[1];";
        Circuit circuit = qasm::parse_source_buffer(qasm);
        CHECK(circuit.size() == 18u);
        CHECK(circuit.num_qubits() == 3u);
    }
}
