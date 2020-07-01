/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Unitary.h"
#include "../../ir/Wire.h"

#include <algorithm>
#include <vector>

namespace tweedledum {

inline bool unitary_verify(Circuit const& right, Circuit const& left,
    double const rtol = 1e-05, double const atol = 1e-08)
{
	Unitary u0("unitary_0");
	std::for_each(right.begin_wire(), right.end_wire(),
	[&](Wire const& wire) {
		u0.create_qubit(wire.name);
	});
	std::for_each(right.begin(), right.end(),
	[&](Instruction const& inst) {
		u0.create_instruction(inst, {inst.begin(), inst.end()});
	});

	Unitary u1("unitary_1");
	std::for_each(left.begin_wire(), left.end_wire(),
	[&](Wire const& wire) {
		u1.create_qubit(wire.name);
	});
	std::for_each(left.begin(), left.end(),
	[&](Instruction const& inst) {
		u1.create_instruction(inst, {inst.begin(), inst.end()});
	});
	return is_approx_equal(u0, u1, rtol, atol);
}

} // namespace tweedledum