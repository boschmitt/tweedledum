/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate.hpp"
#include "../../networks/wire.hpp"

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
	using op_type = typename Network::op_type;
	assert(network.check_gate_set(gate_set::classic_rev));
	assert(network.num_qubits() <= 64);
	network.foreach_op([&](op_type const& op) {
		uint64_t temp_pattern = pattern;
		uint64_t control_mask = 0ull;
		uint64_t target_mask = 0ull;

		switch (op.id()) {
		default:
			std::cerr << "[w] non-classical gate, abort simulation\n";
			pattern = 0ull;
			return false;

		case gate_ids::x:
			pattern ^= (1ull << op.target());
			break;

		case gate_ids::cx:
			if (op.control().is_complemented()) {
				temp_pattern ^= (1ull << op.control());
			}
			if ((temp_pattern >> op.control()) & 1ull) {
				pattern ^= (1ull << op.target());
			}
			break;

		case gate_ids::ncx:
			op.foreach_control([&](wire::id wire) {
				control_mask |= (1ull << wire);
				if (wire.is_complemented()) {
					temp_pattern ^= (1ull << wire);
				}
			});
			op.foreach_target([&](wire::id wire) {
				target_mask |= (1ull << wire);
			});
			if ((temp_pattern & control_mask) == control_mask) {
				pattern ^= target_mask;
			}
			break;
		}
		return true;
	});
	return pattern;
}

} // namespace tweedledum
