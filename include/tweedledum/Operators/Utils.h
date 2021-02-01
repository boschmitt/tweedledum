/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../Utils/Angle.h"
#include "Standard/P.h"
#include "Standard/S.h"
#include "Standard/T.h"
#include "Standard/Z.h"

namespace tweedledum {

inline void apply_identified_phase(Circuit& circuit, Angle angle, WireRef target)
{
    if (!angle.is_numerically_defined()) {
        if (angle == sym_angle::pi_quarter) {
            circuit.apply_operator(Op::T(), {target});
            return;
        }
        if (angle == -sym_angle::pi_quarter) {
            circuit.apply_operator(Op::Tdg(), {target});
            return;
        }
        if (angle == sym_angle::pi_half) {
            circuit.apply_operator(Op::S(), {target});
            return;
        }
        if (angle == -sym_angle::pi_half) {
            circuit.apply_operator(Op::Sdg(), {target});
            return;
        }
        if (angle == sym_angle::pi || angle == -sym_angle::pi) {
            circuit.apply_operator(Op::Z(), {target});
            return;
        }
    }
    circuit.apply_operator(Op::P(angle), {target});
}

inline Angle rotation_angle(Instruction const& inst)
{
    if (inst.is_a<Op::P>()) {
        return inst.cast<Op::P>().angle();
    }
    if (inst.is_a<Op::S>()) {
        return inst.cast<Op::S>().angle();
    }
    if (inst.is_a<Op::Sdg>()) {
        return inst.cast<Op::Sdg>().angle();
    }
    if (inst.is_a<Op::T>()) {
        return inst.cast<Op::T>().angle();
    }
    if (inst.is_a<Op::Tdg>()) {
        return inst.cast<Op::Tdg>().angle();
    }
    if (inst.is_a<Op::Z>()) {
        return inst.cast<Op::Z>().angle();
    }
    return sym_angle::zero;
}

}
