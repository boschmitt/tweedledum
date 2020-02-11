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
	    , num_pauli_x_(0)
	    , num_pauli_y_(0)
	    , num_pauli_z_(0)
	    , num_t_(0)
	    , num_phase_(0)
	{
		num_gates_per_op.fill(0);
		compute_statistics();
	}

	uint32_t num_gates() const
	{
		return this->num_gates();
	}

	uint32_t num_gates(gate_lib operation) const
	{
		const auto op = static_cast<uint32_t>(operation);
		return num_gates_per_op[op];
	}

	uint32_t num_clifford() const
	{
		return num_gates_per_op[static_cast<uint32_t>(gate_lib::cx)]
		       + num_gates_per_op[static_cast<uint32_t>(gate_lib::cz)]
		       + num_gates_per_op[static_cast<uint32_t>(gate_lib::hadamard)]
		       + num_pauli_x_ + num_pauli_y_ + num_pauli_z_
		       + num_phase_;
	}

	// Pauli gates
	uint32_t num_pauli_x() const
	{
		return num_pauli_x_;
	}

	uint32_t num_pauli_y() const
	{
		return num_pauli_y_;
	}

	uint32_t num_pauli_z() const
	{
		return num_pauli_z_;
	}

	// Other phase shift gates
	uint32_t num_t() const
	{
		return num_t_;
	}

	uint32_t num_phase() const
	{
		return num_phase_;
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
			num_phase_ += 1;
			return;
		} else if (rotation_angle == -angles::pi_half) {
			num_phase_ += 1;
			return;
		} else if (rotation_angle == angles::pi) {
			num_pauli_z_ += 1;
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
		switch (numerator) {
		case 6u:
		case 2u:
			num_phase_ += 1;
			break;

		case 4u:
			num_pauli_z_ += 1;
			break;

		case 5u:
			num_pauli_z_ += 1;
		case 7u:
			num_t_ += 1;
			break;

		case 3u:
			num_phase_ += 1;
		case 1u:
			num_t_ += 1;
			break;

		default:
			std::abort();
		}
	}

	void compute_statistics()
	{
		this->foreach_gate([&](auto const& node) {
			if (node.gate.is(gate_lib::rz)) {
				identify_rz(node.gate);
				return;
			}
			if (node.gate.is(gate_lib::rx)) {
				if (node.gate.rotation_angle() == angles::pi) {
					num_pauli_x_ += 1;
					return;
				}
			}
			if (node.gate.is(gate_lib::ry)) {
				if (node.gate.rotation_angle() == angles::pi) {
					num_pauli_y_ += 1;
					return;
				}
			}
			const auto op = static_cast<uint32_t>(node.gate.operation());
			num_gates_per_op[op] += 1;
		});
	}

private:
	static constexpr auto num_defined_ops = static_cast<uint32_t>(gate_lib::num_defined_ops);
	std::array<uint32_t, num_defined_ops> num_gates_per_op;
	uint32_t num_pauli_x_;
	uint32_t num_pauli_y_;
	uint32_t num_pauli_z_;
	uint32_t num_t_;
	uint32_t num_phase_;
};

} // namespace tweedledum
