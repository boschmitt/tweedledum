/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gates/gate.hpp"
#include "../../../networks/wire.hpp"

#include <vector>

namespace tweedledum::detail {

template<typename Network>
void ccccx(Network& network, std::vector<wire::id> const& controls, wire::id  const target)
{
	assert(controls.size() == 4);
	const auto a = controls[0];
	const auto b = controls[1];
	const auto c = controls[2];
	const auto d = controls[3];

	// Find helper qubit
	std::vector<wire::id> workspace;
	network.foreach_wire([&](wire::id wire) {
		if (!wire.is_qubit()) {
			return;
		}
		if (wire == target) {
			return;
		}
		for (wire::id control : controls) {
			if (wire.uid() == control.uid()) {
				return;
			}
		}
		workspace.push_back(wire);
	});
	assert(workspace.size());
	wire::id helper = workspace.back();

	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, c, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::cx, a, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, b, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, a, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, b, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, c, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::h, target);
	network.create_op(gate_lib::cx, target, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, d, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, target, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, d, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, c, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, b, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, a, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, b, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, a, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, c, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::h, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, d, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, target, helper);
	network.create_op(gate_lib::tdg, helper);
	network.create_op(gate_lib::cx, d, helper);
	network.create_op(gate_lib::t, helper);
	network.create_op(gate_lib::cx, target, helper);
	network.create_op(gate_lib::h, target);
}

} // namespace tweedledum::detail