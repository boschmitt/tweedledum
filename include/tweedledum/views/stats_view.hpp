/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_lib.hpp"
#include "../traits.hpp"
#include "immutable_view.hpp"

#include <array>
#include <fmt/format.h>
#include <iostream>

namespace tweedledum {

/*! \brief
 */
template<typename Network>
class stats_view : public immutable_view<Network> {
public:
	using gate_type = typename Network::gate_type;
	using node_type = typename Network::node_type;
	using link_type = typename Network::link_type;
	using storage_type = typename Network::storage_type;

	explicit stats_view(Network& ntk)
	    : immutable_view<Network>(ntk)
	    , num_clifford_(0)
	    , num_t_(0)
	{
		num_gates_per_op.fill(0);
		compute_statistics();
	}

	uint32_t num_gates(gate_lib operation)
	{
		const auto op = static_cast<uint32_t>(operation);
		return num_gates_per_op[op];
	}

	uint32_t num_clifford()
	{
		return num_clifford_;
	}

	uint32_t num_t()
	{
		return num_t_;
	}

private:
	void identify_rz(gate_type const& gate)
	{
		angle rotation_angle = gate.rotation_angle();
		if (rotation_angle.is_numerically_defined()) {
			const auto op = static_cast<uint32_t>(gate.operation());
			num_gates_per_op[op] += 1;
			return;
		}
		// Try to identify gates based on known rotations
		if (rotation_angle == angles::pi_quarter) {
			num_t_ += 1;
			return;
		} else if (rotation_angle == -angles::pi_quarter) {
			num_t_ += 1;
			return;
		} else if (rotation_angle == angles::pi_half) {
			num_clifford_ += 1;
			return;
		} else if (rotation_angle == -angles::pi_half) {
			num_clifford_ += 1;
			return;
		} else if (rotation_angle == angles::pi) {
			num_clifford_ += 1;
			return;
		}

		// Try to identify gates based composition known rotations
		auto [numerator, denominator] = rotation_angle.symbolic_value().value();
		if ((4 % denominator) != 0) {
			return;
		}
		numerator *= (4 / denominator);
		if (numerator < 0) {
			numerator = 8 + numerator;
		}
		assert(numerator > 0 && numerator < 8);
		if ((numerator & 1) == 0) {
			num_clifford_ += 1;
		} else {
			if (numerator == 5 || numerator == 3) {
				num_clifford_ += 1;
			}
			num_t_ += 1;
		}
	}

	void compute_statistics()
	{
		this->foreach_gate([&](auto const& node) {
			if (node.gate.is(gate_lib::rotation_z)) {
				identify_rz(node.gate);
				return;
			}
			const auto op = static_cast<uint32_t>(node.gate.operation());
			num_gates_per_op[op] += 1;
		});
	}

private:
	static constexpr auto num_defined_ops = static_cast<uint32_t>(gate_lib::num_defined_ops);
	std::array<uint32_t, num_defined_ops> num_gates_per_op;
	uint32_t num_clifford_;
	uint32_t num_t_;
};

} // namespace tweedledum
