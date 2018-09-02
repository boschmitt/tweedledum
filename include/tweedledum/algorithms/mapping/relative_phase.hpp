/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"
#include "../generic/rewrite.hpp"

namespace tweedledum {

template<class NetworkDest, class NetworkSrc>
void relative_phase_mapping(NetworkDest& dest, NetworkSrc const& src)
{
	rewrite_network(dest, src, [](auto& dest, auto const& g) {
		if (g.is(gate_kinds_t::mcx)) {
			switch (g.num_controls()) {
			default:
				return false;
			case 0:
				g.foreach_target([&](auto t) {
					dest.add_gate(gate_kinds_t::pauli_x, t);
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
			case 2: {
				uint32_t controls[2];
				auto* p = controls;
				g.foreach_control([&](auto c) { *p++ = c; });

				std::vector<uint32_t> targets(g.num_targets());
				auto it = targets.begin();
				g.foreach_target([&](auto t) { *it++ = t; });

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_controlled_gate(gate_kinds_t::cx,
					                         targets[0],
					                         targets[i]);
				}

				const auto a = controls[0];
				const auto b = controls[1];
				const auto c = targets[0];

				dest.add_gate(gate_kinds_t::hadamard, c);

				dest.add_controlled_gate(gate_kinds_t::cx, b, c);
				dest.add_gate(gate_kinds_t::t_dagger, c);
				dest.add_controlled_gate(gate_kinds_t::cx, a, c);
				dest.add_gate(gate_kinds_t::t, c);
				dest.add_controlled_gate(gate_kinds_t::cx, b, c);
				dest.add_gate(gate_kinds_t::t_dagger, c);
				dest.add_controlled_gate(gate_kinds_t::cx, a, c);
				dest.add_gate(gate_kinds_t::t, c);
				dest.add_controlled_gate(gate_kinds_t::cx, a, b);
				dest.add_gate(gate_kinds_t::t_dagger, b);
				dest.add_controlled_gate(gate_kinds_t::cx, a, b);
				dest.add_gate(gate_kinds_t::t, b);
				dest.add_gate(gate_kinds_t::t, a);

				dest.add_gate(gate_kinds_t::hadamard, c);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_controlled_gate(gate_kinds_t::cx,
					                         targets[0],
					                         targets[i]);
				}
			} break;
			case 3: {
				uint32_t controls[3];
				auto* p = controls;
				g.foreach_control([&](auto c) { *p++ = c; });

				std::vector<uint32_t> targets(g.num_targets());
				auto it = targets.begin();
				g.foreach_target([&](auto t) { *it++ = t; });

				const auto a = controls[0];
				const auto b = controls[1];
				const auto c = controls[2];
				const auto d = targets[0];

				int32_t hl = -1;
				for (auto i = 0u; i < dest.num_qubits(); ++i) {
					if (i != a && i != b && i != c
					    && (std::find(targets.begin(),
					                  targets.end(), i)
					        == targets.end())) {
						hl = i;
						break;
					}
				}

				if (hl == -1) {
					std::cout
					    << "[e] no sufficient helper line "
					       "found for mapping, break\n";
					return false;
				}

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_controlled_gate(
					    gate_kinds_t::cx, d, targets[i]);
				}

				// R1-TOF(a, b, hl)
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, a, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);

				// S-R2-TOF(c3, hl, target)
				dest.add_gate(gate_kinds_t::hadamard, d);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t, hl);

				// R1-TOF^-1(a, b, hl)
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, a, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);

				// S-R2-TOF^-1(c3, hl, target)
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::hadamard, d);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_controlled_gate(
					    gate_kinds_t::cx, d, targets[i]);
				}
			} break;

			case 4u:
			{
				uint32_t controls[4];
				auto* p = controls;
				g.foreach_control([&](auto c) { *p++ = c; });

				std::vector<uint32_t> targets(g.num_targets());
				auto it = targets.begin();
				g.foreach_target([&](auto t) { *it++ = t; });

				const auto a = controls[0];
				const auto b = controls[1];
				const auto c = controls[2];
				const auto d = controls[3];
				const auto e = targets[0];

				int32_t hl = -1;
				for (auto i = 0u; i < dest.num_qubits(); ++i) {
					if (i != a && i != b && i != c && i != d
					    && (std::find(targets.begin(),
					                  targets.end(), i)
					        == targets.end())) {
						hl = i;
						break;
					}
				}

				if (hl == -1) {
					std::cout
					    << "[e] no sufficient helper line "
					       "found for mapping, break\n";
					return false;
				}

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_controlled_gate(
					    gate_kinds_t::cx, e, targets[i]);
				}

				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, a, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, a, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::hadamard, e);
				dest.add_controlled_gate(gate_kinds_t::cx, e, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, e, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, a, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, b, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, a, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, c, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_gate(gate_kinds_t::hadamard, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, e, hl);
				dest.add_gate(gate_kinds_t::t_dagger, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, d, hl);
				dest.add_gate(gate_kinds_t::t, hl);
				dest.add_controlled_gate(gate_kinds_t::cx, e, hl);
				dest.add_gate(gate_kinds_t::hadamard, e);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_controlled_gate(
					    gate_kinds_t::cx, e, targets[i]);
				}
			} break;
			}

			return true;
		}

		return false;
	}, 1);
}

} // namespace tweedledum
