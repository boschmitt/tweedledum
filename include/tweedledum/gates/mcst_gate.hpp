/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../traits.hpp"
#include "gate_kinds.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <type_traits>
#include <vector>

namespace tweedledum {

/*! \brief Multiple Control Single Target quantum gate.
 *
 * Must have one target.
 * Qubits are sorted
 */
class mcst_gate {
public:
#pragma region Constants
	static constexpr uint32_t max_num_qubits = 3u;
#pragma endregion

#pragma region Constructors
	mcst_gate(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0)
	    : kind_(static_cast<uint32_t>(kind))
	    , target_(0)
	    , rotation_angle_(rotation_angle)
	    , qubits_({target, invalid_qid, invalid_qid})
	{}

	mcst_gate(gate_kinds_t kind, uint32_t control, uint32_t target, float rotation_angle = 0.0)
	    : kind_(static_cast<uint32_t>(kind))
	    , target_(0)
	    , rotation_angle_(rotation_angle)
	    , qubits_({target, control, invalid_qid})
	{
		assert(control != target);
		if (control < target) {
			std::swap(qubits_[0], qubits_[1]);
			target_ = 1;
		}
	}

	mcst_gate(gate_kinds_t kind, std::vector<uint32_t> const& controls,
	          std::vector<uint32_t> const& targets, float rotation_angle = 0.0)
	    : kind_(static_cast<uint32_t>(kind))
	    , target_(0)
	    , rotation_angle_(rotation_angle)
	    , qubits_({invalid_qid, invalid_qid, invalid_qid})
	{
		assert(controls.size() <= 2);
		assert(targets.size() == 1);
		for (auto i = 0u; i < controls.size(); ++i) {
			qubits_[i] = controls[i];
		}
		qubits_[2] = targets[0];
		std::sort(qubits_.begin(), qubits_.end());
		for (auto i = 0u; i < qubits_.size(); ++i) {
			if (qubits_[i] == targets[0]) {
				target_ = i;
				break;
			}
		}
	}
#pragma endregion

#pragma region Properties
	auto num_controls() const
	{
		if (kind_ < static_cast<uint32_t>(gate_kinds_t::cx)) {
			return 0u;
		}
		auto num_controls = 1u;
		if (this->is_one_of(gate_kinds_t::mcx, gate_kinds_t::mcz)) {
			++num_controls;
		}
		return num_controls;
	}

	auto num_targets() const
	{
		return 1u;
	}

	auto kind() const
	{
		return static_cast<gate_kinds_t>(kind_);
	}

	bool is(gate_kinds_t kind) const
	{
		return kind_ == static_cast<uint32_t>(kind);
	}

	template<typename... Ts>
	bool is_one_of(gate_kinds_t t) const
	{
		return is(t);
	}

	template<typename... Ts>
	bool is_one_of(gate_kinds_t t0, Ts... tn) const
	{
		return is(t0) || is_one_of(tn...);
	}

	bool is_z_rotation() const
	{
		return is_one_of(gate_kinds_t::phase, gate_kinds_t::phase_dagger, gate_kinds_t::t,
		                 gate_kinds_t::t_dagger, gate_kinds_t::pauli_z,
		                 gate_kinds_t::rotation_z, gate_kinds_t::cz, gate_kinds_t::mcz);
	}

	bool is_x_rotation() const
	{
		return is_one_of(gate_kinds_t::pauli_x, gate_kinds_t::rotation_x, gate_kinds_t::cx,
		                 gate_kinds_t::mcx);
	}

	bool is_dependent(mcst_gate const& other) const
	{
		if (*this == other) {
			return false;
		}
		if (this->is_z_rotation()) {
			if (other.is_z_rotation()) {
				return false;
			}
			if (other.is_x_rotation()) {
				// Check if the target of the 'other' gate affects the controls
				// of 'this' gate
				for (auto i = 0u; i < max_num_qubits; ++i) {
					if (i != target_
					    && qubits_[i] == other.qubits_[other.target_]) {
						return true;
					}
				}
				return qubits_[target_] == other.qubits_[other.target_];
			}
		}
		if (this->is_x_rotation()) {
			// Check if the target of the 'this' gate affects the controls
			// of 'other' gate
			for (auto i = 0u; i < max_num_qubits; ++i) {
				if (i != other.target_ && other.qubits_[i] == qubits_[target_]) {
					return true;
				}
			}
			if (other.is_z_rotation()) {
				return qubits_[target_] == other.qubits_[other.target_];
			}
			if (other.is_x_rotation()) {
				// Check if the target of the 'other' gate affects the controls
				// of 'this' gate
				for (auto i = 0u; i < max_num_qubits; ++i) {
					if (i != target_
					    && qubits_[i] == other.qubits_[other.target_]) {
						return true;
					}
				}
				return false;
			}
		}
		return true;
	}

	auto is_control(uint32_t qid) const
	{
		for (auto i = 0u; i < max_num_qubits; ++i) {
			if (qubits_[i] == qid && i != target_) {
				return true;
			}
		}
		return false;
	}

	auto rotation_angle() const
	{
		return rotation_angle_;
	}

	auto qubit_index(uint32_t qid) const
	{
		for (auto i = 0u; i < qubits_.size(); ++i) {
			if (qubits_[i] == qid) {
				return i;
			}
		}
		assert(0);
		return static_cast<uint32_t>(qubits_.size());
	}
#pragma endregion

#pragma region Iterators
	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		fn(qubits_[target_]);
	}

	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		if (kind_ < static_cast<uint32_t>(gate_kinds_t::cx)) {
			return;
		}
		for (auto i = 0u; i < max_num_qubits; ++i) {
			if (i == target_ || qubits_[i] == invalid_qid) {
				continue;
			}
			if constexpr (std::is_invocable_r<bool, Fn, uint32_t>::value) {
				if (!fn(qubits_[i])) {
					return;
				}
			} else {
				fn(qubits_[i]);
			}
		}
	}
#pragma endregion

#pragma region Overloads
	bool operator==(mcst_gate const& other) const
	{
		if (kind() != other.kind()) {
			return false;
		}
		if (target_ != other.target_) {
			return false;
		}
		for (auto i = 0u; i < max_num_qubits; ++i) {
			if (qubits_[i] != other.qubits_[i]) {
				return false;
			}
		}
		return true;
	}
#pragma endregion

#pragma region Debug
	void print(std::ostream& out = std::cout) const
	{
		out << fmt::format("Gate: {}\n", gate_name(kind()));
	}

	friend std::ostream& operator<<(std::ostream& out, mcst_gate const& gate)
	{
		out << fmt::format("(Name: {}, ", gate_name(gate.kind()));
		out << fmt::format("Qubits:");
		gate.foreach_control([&out](auto qid) { out << " " << qid; });
		gate.foreach_target([&out](auto qid) { out << " " << qid; });
		out << ')';
		return out;
	}
#pragma endregion

private:
	uint32_t kind_ : 16;
	uint32_t target_ : 16;
	float rotation_angle_;
	std::array<uint32_t, max_num_qubits> qubits_;
};

} // namespace tweedledum
