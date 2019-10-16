/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_lib.hpp"
#include "../../networks/io_id.hpp"
#include "../../utils/angle.hpp"

#include <cstdint>
#include <iostream>

namespace tweedledum {

/*! \brief Simulate a quantum circuit that has only classical gates.
 *
 * \algtype simulation
 * \algexpects A Toffoli network
 * \algreturns The simulated pattern
 */
// TODO: make it aware of rewiring
template<typename Network>
uint64_t simulate_classically(Network const& network, uint64_t pattern)
{
	assert(network.num_qubits() <= 64);
	network.foreach_gate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			std::cerr << "[w] non-classical gate, abort simulation\n";
			pattern = 0ull;
			return false;

		case gate_lib::rotation_x: {
			angle rotation_angle = gate.rotation_angle();
			if (rotation_angle == angles::pi) {
				gate.foreach_target([&](io_id id) {
					pattern ^= (1ull << id);
				});
			} else {
				std::cerr << "[w] unsupported gate type\n";
				assert(0);
			}
		} break;

		case gate_lib::cx:
			gate.foreach_control([&](io_id control_id) {
				uint64_t temp_pattern = pattern;
				if (control_id.is_complemented()) {
					temp_pattern ^= (1ull << control_id);
				}
				gate.foreach_target([&](io_id target_id) {
					if ((temp_pattern >> control_id) & 1ull) {
						pattern ^= (1ull << target_id);
					}
				});
			});
			break;

		case gate_lib::mcx: {
			uint64_t control_mask = 0;
			uint64_t target_mask = 0;
			uint64_t temp_pattern = pattern;
			gate.foreach_control([&](io_id id) {
				control_mask |= (1ull << id);
				if (id.is_complemented()) {
					temp_pattern ^= (1ull << id);
				}
			});
			gate.foreach_target([&](io_id id) {
				target_mask |= (1ull << id);
			});
			if ((temp_pattern & control_mask) == control_mask) {
				pattern ^= target_mask;
			}
		} break;
		}
		return true;
	});
	return pattern;
}

} // namespace tweedledum
