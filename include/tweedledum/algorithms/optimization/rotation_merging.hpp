/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

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
 *
 * **Required network functions:**
 */
template<typename NetworkSrc, typename NetworkDest>
void rotation_merging(NetworkSrc const& src, NetworkDest& dest)
{
	using term_type = typename pathsum_view<NetworkSrc>::esop_type;
	parity_terms<term_type> parities;

	pathsum_view pathsums(src);
	// Go thought the networ and merge angles of rotations being
	// applied to the same pathsum
	src.foreach_gate([&](auto const& node) {
		if (!node.gate.is_z_rotation()) {
			return;
		}
		auto term = pathsums.get_pathsum(node);
		parities.add_term(term, node.gate.rotation_angle());
	});

	src.foreach_gate([&](auto const& node) {
		if (node.gate.is_z_rotation()) {
			return;
		}
		dest.emplace_gate(node.gate);
		auto angle = parities.extract_term(pathsums.get_pathsum(node));
		if (angle != angles::zero) {
			dest.add_gate(gate_base(gate_set::rotation_z, angle), node.gate.target());
		}
	});
}

/*! \brief TODO
 *
 * **Required network functions:**
 */
template<typename Network>
Network rotation_merging(Network const& src)
{
	Network dest;
	src.foreach_io([&](io_id io, std::string const& label) {
		if (io.is_qubit()) {
			dest.add_qubit(label);
		} else {
			dest.add_cbit(label);
		}
	});
	rotation_merging(src, dest);
	return dest;
}

} // namespace tweedledum
