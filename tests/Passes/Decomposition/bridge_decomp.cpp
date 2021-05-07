/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/bridge_decomp.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Extension/Bridge.h"
#include "tweedledum/Target/Device.h"

#include "../check_unitary.h"

#include <catch.hpp>

using namespace tweedledum;

inline bool check_decomp(Circuit const& left, Circuit const& right,
  double const rtol = 1e-05, double const atol = 1e-08)
{
    Op::UnitaryBuilder left_unitary(left.num_qubits());
    left.foreach_instruction([&](Instruction const& inst) {
        left_unitary.apply_operator(inst, inst.qubits());
    });
    Op::Unitary u_left = left_unitary.finished();

    Op::UnitaryBuilder right_unitary(right.num_qubits());
    right.foreach_instruction([&](Instruction const& inst) {
        right_unitary.apply_operator(inst, inst.qubits());
    });
    Op::Unitary u_right = right_unitary.finished();

    uint32_t const size = (1 << left.num_qubits());
    Op::Unitary t(u_right.matrix().block(0, 0, size, size));
    return is_approx_equal(u_left, t, rtol, atol);
}

TEST_CASE("Test bridges on lines", "[bridge_decomp][decomp]")
{
    using namespace tweedledum;
    for (uint32_t dist = 2u; dist < 7u; ++dist) {
        Circuit original;
        for (uint32_t i = 0; i < (dist + 1); ++i) {
            original.create_qubit();
        }
        Device device = Device::path(original.num_qubits());
        original.apply_operator(Op::Bridge(), {Qubit(dist), Qubit(0)});
        Circuit decomposed = bridge_decomp(device, original);
        CHECK(check_decomp(original, decomposed));
    }
}
