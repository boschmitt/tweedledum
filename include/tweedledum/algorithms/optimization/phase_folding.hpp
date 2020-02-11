/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../generic/shallow_duplicate.hpp"
#include "../../networks/io_id.hpp"
#include "../../utils/angle.hpp"
#include "../../utils/parity_terms.hpp"
#include "../../views/pathsum_view.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <vector>

namespace tweedledum {

/*! \brief TODO
 */
template<typename Network>
Network phase_folding(Network const& network)
{
	using term_type = typename pathsum_view<Network>::esop_type;
	Network result = shallow_duplicate(network);
	parity_terms<term_type> parities;

	pathsum_view pathsums(network);
	// Go through the network and merge angles of rotations being applied to the same pathsum.
	network.foreach_gate([&](auto const& node) {
		if (!node.gate.is_z_rotation()) {
			return;
		}
		auto term = pathsums.get_pathsum(node);
		parities.add_term(term, node.gate.rotation_angle());
	});

	network.foreach_gate([&](auto const& node) {
		if (node.gate.is_z_rotation()) {
			return;
		}
		result.emplace_gate(node.gate);
		auto const term = pathsums.get_pathsum(node);
		auto angle = parities.extract_term(term);
		if (angle == angles::zero) {
			return;
		}
		result.add_gate(gate_base(gate_lib::rz, angle), node.gate.target());
	});
	result.rewire(network.wiring_map());
	return result;
}

} // namespace tweedledum
