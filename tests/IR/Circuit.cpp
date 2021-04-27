/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Circuit.h"

#include <catch.hpp>

TEST_CASE("Simple circuit functionality", "[circuit][ir]")
{
    using namespace tweedledum;
    Circuit circuit;

    Cbit cbit = circuit.create_cbit();
    Qubit qubit = circuit.create_qubit();
    CHECK(circuit.num_cbits() == 1u);
    CHECK(circuit.num_qubits() == 1u);
    CHECK(static_cast<uint32_t>(cbit) == static_cast<uint32_t>(qubit));
    // CHECK(qubit.kind() == Wire::Kind::quantum);
    // CHECK(cbit.kind() == Wire::Kind::classical);
}

