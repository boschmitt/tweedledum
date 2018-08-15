/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"
#include "../generic/rewrite.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace tweedledum {

namespace detail {

template<class Network>
void nct_insert_toffoli(Network& net, std::vector<uint32_t> const& controls,
                        uint32_t target)
{
	const auto c = controls.size();
	assert(c >= 2);

	if (c == 2) {
		net.add_multiple_controlled_gate(
		    gate_kinds_t::mcx, {target, controls[0], controls[1]});
		return;
	}

	/* enough empty lines? */
	if (net.num_qubits() + 1 >= (c << 1)) {
		/* empty lines */
		std::vector<uint32_t> empty;
		for (auto i = 0u; i < net.num_qubits(); ++i) {
			if (i != target
			    && std::find(controls.begin(), controls.end(), i)
			           == controls.end()) {
				empty.push_back(i);
			}
		}

		const auto e = empty.size();
		empty.push_back(target);

		for (auto offset = 0u; offset <= 1u; ++offset) {
			for (auto i = offset; i < c - 2u; ++i) {
				net.add_multiple_controlled_gate(
				    gate_kinds_t::mcx,
				    {empty[e - i], controls[c - 1 - i],
				     empty[e - 1 - i]});
			}
			net.add_multiple_controlled_gate(
			    gate_kinds_t::mcx,
			    {empty[e - (c - 2)], controls[0], controls[1]});
			for (auto i = offset; i < c - 2u; ++i) {
				net.add_multiple_controlled_gate(
				    gate_kinds_t::mcx,
				    {empty[e - i], controls[c - 1 - i],
				     empty[e - 1 - i]});
			}
		}

		return;
	}

	/* not enough empty lines, extra decomposition step */
	int32_t e{-1};
	for (auto i = 0u; i < net.num_qubits(); ++i) {
		if (i != target
		    && std::find(controls.begin(), controls.end(), i)
		           == controls.end()) {
			e = -1;
			break;
		}
	}
	if (e == -1) {
		std::cout << "[e] no sufficient helper line found for mapping, "
		             "break\n";
		return;
	}

	std::vector<uint32_t> c1, c2;
	const auto m = c >> 1;
	for (auto i = 0u; i < m; ++i) {
		c1.push_back(controls[i]);
	}
	for (auto i = m; i < c; ++i) {
		c2.push_back(controls[i]);
	}
	c2.push_back(controls[e]);
	nct_insert_toffoli(net, c1, e);
	nct_insert_toffoli(net, c2, target);
	nct_insert_toffoli(net, c1, e);
	nct_insert_toffoli(net, c2, target);
}

} /* namespace detail */

template<class NetworkDest, class NetworkSrc>
void nct_mapping(NetworkDest& dest, NetworkSrc const& src)
{
	rewrite_network(
	    dest, src,
	    [](auto& dest, auto const& g) {
		    if (g.is(gate_kinds_t::mcx)) {
			    switch (g.num_controls()) {
			    case 0:
				    g.foreach_target([&](auto t) {
					    dest.add_gate(gate_kinds_t::pauli_x,
					                  t);
				    });
				    break;
			    case 1:
				    g.foreach_control([&](auto c) {
					    g.foreach_target([&](auto t) {
						    dest.add_controlled_gate(
						        gate_kinds_t::cx, c, t);
					    });
				    });
				    break;
			    default: {
				    std::vector<uint32_t> controls, targets;
				    g.foreach_control(
				        [&](auto c) { controls.push_back(c); });
				    g.foreach_target(
				        [&](auto t) { targets.push_back(t); });
				    for (auto i = 1u; i < targets.size(); ++i)
					    dest.add_controlled_gate(
					        gate_kinds_t::cx, targets[0],
					        targets[i]);
				    detail::nct_insert_toffoli(dest, controls,
				                               targets[0]);
				    for (auto i = 1u; i < targets.size(); ++i)
					    dest.add_controlled_gate(
					        gate_kinds_t::cx, targets[0],
					        targets[i]);
			    } break;
			    }

			    return true;
		    }

		    return false;
	    },
	    1);
}

} // namespace tweedledum