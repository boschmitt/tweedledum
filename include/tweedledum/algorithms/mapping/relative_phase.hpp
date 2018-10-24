/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_kinds.hpp"
#include "../../traits.hpp"
#include "../generic/rewrite.hpp"

#include <iostream>

namespace tweedledum {

/*! \brief
 *
 * **Required gate functions:**
 *
 * **Required network functions:**
 */
template<typename NetworkDest, typename NetworkSrc>
NetworkDest relative_phase_mapping(NetworkSrc const& src)
{
	auto gate_rewriter = [](auto& dest, auto const& gate) {
		if (gate.is(gate_kinds_t::mcx)) {
			switch (gate.num_controls()) {
			default:
				return false;

			case 0u:
				gate.foreach_target([&](auto target) {
					dest.add_gate(gate_kinds_t::pauli_x, target);
				});
				break;

			case 1u:
				gate.foreach_control([&](auto control) {
					gate.foreach_target([&](auto target) {
						dest.add_gate(gate_kinds_t::cx, control, target);
					});
				});
				break;

			case 2u: {
				uint32_t controls[2];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, targets[0], targets[i]);
				}
				dest.add_gate(gate_kinds_t::hadamard, targets[0]);

				dest.add_gate(gate_kinds_t::cx, controls[1], targets[0]);
				dest.add_gate(gate_kinds_t::t_dagger, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[0], targets[0]);
				dest.add_gate(gate_kinds_t::t, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[1], targets[0]);
				dest.add_gate(gate_kinds_t::t_dagger, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[0], targets[0]);
				dest.add_gate(gate_kinds_t::t, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[0], controls[1]);
				dest.add_gate(gate_kinds_t::t_dagger, controls[1]);
				dest.add_gate(gate_kinds_t::cx, controls[0], controls[1]);
				dest.add_gate(gate_kinds_t::t, controls[1]);
				dest.add_gate(gate_kinds_t::t, controls[0]);

				dest.add_gate(gate_kinds_t::hadamard, targets[0]);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, targets[0], targets[i]);
				}
			} break;

			case 3u: {
				uint32_t controls[3];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				const auto a = controls[0];
				const auto b = controls[1];
				const auto c = controls[2];
				const auto d = targets[0];
				auto helper = invalid_qid;
				for (auto i = 0u; i < dest.num_qubits(); ++i) {
					if (i != a && i != b && i != c
					    && (std::find(targets.begin(), targets.end(), i)
					        == targets.end())) {
						helper = i;
						break;
					}
				}

				if (helper == invalid_qid) {
					std::cout << "[e] insufficient helper qubits for mapping, "
					             "break\n";
					return false;
				}

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, d, targets[i]);
				}

				// R1-TOF(a, b, helper)
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, a, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);

				// S-R2-TOF(c3, helper, target)
				dest.add_gate(gate_kinds_t::hadamard, d);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, c, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, c, helper);
				dest.add_gate(gate_kinds_t::t, helper);

				// R1-TOF^-1(a, b, helper)
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, a, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);

				// S-R2-TOF^-1(c3, helper, target)
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, c, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, c, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::hadamard, d);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, d, targets[i]);
				}
			} break;

			case 4u: {
				uint32_t controls[4];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				const auto a = controls[0];
				const auto b = controls[1];
				const auto control = controls[2];
				const auto d = controls[3];
				const auto e = targets[0];

				auto helper = invalid_qid;
				for (auto i = 0u; i < dest.num_qubits(); ++i) {
					if (i != a && i != b && i != control && i != d
					    && (std::find(targets.begin(), targets.end(), i)
					        == targets.end())) {
						helper = i;
						break;
					}
				}

				if (helper == invalid_qid) {
					std::cout << "[e] no sufficient helper line "
					             "found for mapping, break\n";
					return false;
				}

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, e, targets[i]);
				}

				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, control, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::cx, a, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, a, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, control, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::hadamard, e);
				dest.add_gate(gate_kinds_t::cx, e, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, e, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, control, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, a, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, b, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, a, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, control, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::hadamard, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, e, helper);
				dest.add_gate(gate_kinds_t::t_dagger, helper);
				dest.add_gate(gate_kinds_t::cx, d, helper);
				dest.add_gate(gate_kinds_t::t, helper);
				dest.add_gate(gate_kinds_t::cx, e, helper);
				dest.add_gate(gate_kinds_t::hadamard, e);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_kinds_t::cx, e, targets[i]);
				}
			} break;
			}
			return true;
		} else if (gate.is(gate_kinds_t::mcz)) {
			if (gate.num_controls() == 2) {
				uint32_t controls[2];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				dest.add_gate(gate_kinds_t::cx, controls[1], targets[0]);
				dest.add_gate(gate_kinds_t::t_dagger, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[0], targets[0]);
				dest.add_gate(gate_kinds_t::t, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[1], targets[0]);
				dest.add_gate(gate_kinds_t::t_dagger, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[0], targets[0]);
				dest.add_gate(gate_kinds_t::t, targets[0]);
				dest.add_gate(gate_kinds_t::cx, controls[0], controls[1]);
				dest.add_gate(gate_kinds_t::t_dagger, controls[1]);
				dest.add_gate(gate_kinds_t::cx, controls[0], controls[1]);
				dest.add_gate(gate_kinds_t::t, controls[1]);
				dest.add_gate(gate_kinds_t::t, controls[0]);
				return true;
			}
		}
		return false;
	};

	constexpr auto num_ancillae = 1u;
	NetworkDest dest;
	rewrite_network(dest, src, gate_rewriter, num_ancillae);
	return dest;
}

} // namespace tweedledum
