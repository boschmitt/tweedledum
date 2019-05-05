/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../identify_rz.hpp"
#include "../../networks/io_id.hpp"
#include "../../utils/angle.hpp"
#include "../../utils/parity_terms.hpp"
#include "../../views/pathsum_view.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <vector>

namespace tweedledum {

struct phase_folding_params
{
        bool use_generic_rx = false;
};

/*! \brief TODO
 *
 * **Required network functions:**
 */
template<typename NetworkSrc, typename NetworkDest>
void phase_folding(NetworkSrc const& src, NetworkDest& dest, phase_folding_params params = {})
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
		// auto b = parities.num_terms();
		// angle a = parities.extract_term(term);
		// angle b = angle(2, 1); 
		// if (a == b) {
		// 	std::cout << "eureka!\n";
		// 	std::cout << a << " " << b << "\n\n";
		// 	return;
		// } 
		// parities.add_term(term, a);
	});

	src.foreach_gate([&](auto const& node) {
		if (node.gate.is_z_rotation()) {
			return;
		}
		dest.emplace_gate(node.gate);
		auto angle = parities.extract_term(pathsums.get_pathsum(node));
		if (angle == angles::zero) {
			return;
		}
		dest.add_gate(gate_base(gate_set::rotation_z, angle), node.gate.target());
	});
	if (params.use_generic_rx == false) {
		dest = identify_rz(dest);
	}
}

/*! \brief TODO
 *
 * **Required network functions:**
 */
template<typename Network>
Network phase_folding(Network const& src, phase_folding_params params = {})
{
	Network dest;
	src.foreach_io([&](io_id io, std::string const& label) {
		if (io.is_qubit()) {
			dest.add_qubit(label);
		} else {
			dest.add_cbit(label);
		}
	});
	phase_folding(src, dest, params);
	return dest;
}

} // namespace tweedledum
