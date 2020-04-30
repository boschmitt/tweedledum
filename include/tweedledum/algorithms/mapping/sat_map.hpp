/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/mapped_dag.hpp"
#include "../../networks/wire.hpp"
#include "../../target/device.hpp"
#include "placement/sat_placement.hpp"

namespace tweedledum {

/*! \brief Yet to be written.
 */
template<typename Circuit>
mapped_dag sat_map(Circuit const& original, device const& device)
{
	using op_type = typename Circuit::op_type;
	mapped_dag mapped(original, device);
	
	std::vector<wire::id> v_to_phy = detail::sat_place(original, device);
	if (v_to_phy.empty()) {
		return mapped;
	}

	std::vector<wire::id> wire_to_v(original.num_wires(), wire::invalid_id);
	original.foreach_wire([&](wire::id wire, std::string_view name) {
		wire_to_v.at(wire) = mapped.wire(name);
	});

	mapped.v_to_phy(v_to_phy);
	original.foreach_op([&](op_type const& op) {
		wire::id const phy0 = v_to_phy.at(wire_to_v.at(op.target()));
		if (op.is_one_qubit()) {
			mapped.create_op(op, phy0);
		} else if (op.is_two_qubit()) {
			wire::id const phy1 = v_to_phy.at(wire_to_v.at(op.control()));
			mapped.create_op(op, phy1, phy0);
		}
	});
	return mapped;
}

} // namespace tweedledum
