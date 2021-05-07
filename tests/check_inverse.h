/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Extension/Unitary.h"

using namespace tweedledum;

inline bool check_inverse(
  Circuit const& left, Circuit const& right, double const atol = 1e-08)
{
    Op::UnitaryBuilder left_unitary(left.num_qubits(), left.global_phase());
    left.foreach_instruction([&](Instruction const& inst) {
        left_unitary.apply_operator(inst, inst.qubits());
    });
    Op::Unitary u_left = left_unitary.finished();

    Op::UnitaryBuilder right_unitary(right.num_qubits(), right.global_phase());
    right.foreach_instruction([&](Instruction const& inst) {
        right_unitary.apply_operator(inst, inst.qubits());
    });
    Op::Unitary u_right = right_unitary.finished();

    UMatrix result = u_left.matrix() * u_right.matrix();
    return result.isIdentity(atol);
}