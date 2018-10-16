/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "gate_kinds.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <stack>
#include <vector>

namespace tweedledum {

/*! \brief Multiple Control Multiple Target reversible gate.
 *
 * This gate type needs the network itself to have a maximum number o qubits of 32 (need to figure
 * out how to do this)
 */
class mcmt_gate {
public:
#pragma region Constants
	static constexpr uint32_t max_num_qubits = 32u;
	static constexpr uint32_t network_max_num_qubits = 32u;
#pragma endregion

#pragma region Constructors
	mcmt_gate(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0)
	    : kind_(static_cast<uint32_t>(kind))
	    , controls_(0)
	    , targets_(0)
	    , rotation_angle_(rotation_angle)
	{
		assert(target <= 32);
		targets_ |= (1 << target);
	}

	mcmt_gate(gate_kinds_t kind, uint32_t control, uint32_t target, float rotation_angle = 0.0)
	    : kind_(static_cast<uint32_t>(kind))
	    , controls_(0)
	    , targets_(0)
	    , rotation_angle_(rotation_angle)
	{
		assert(target <= 32);
		assert(control <= 32);
		targets_ |= (1 << target);
		controls_ |= (1 << control);
	}

	mcmt_gate(gate_kinds_t kind, std::vector<uint32_t> controls, std::vector<uint32_t> targets,
	          float rotation_angle = 0.0)
	    : kind_(static_cast<uint32_t>(kind))
	    , controls_(0)
	    , targets_(0)
	    , rotation_angle_(rotation_angle)
	{
		assert(controls.size() >= 0 && controls.size() <= 32);
		assert(targets.size() > 0 && targets.size() <= 32);
		for (auto control : controls) {
			assert(control <= 32u);
			controls_ |= (1u << control);
		}
		for (auto target : targets) {
			assert(target <= 32u);
			targets_ |= (1u << target);
		}
		assert((targets_ & controls_) == 0u);
	}
#pragma endregion

#pragma region Properties
	auto num_controls() const
	{
		return __builtin_popcount(controls_);
	}

	auto num_targets() const
	{
		return __builtin_popcount(targets_);
	}

	auto kind() const
	{
		return static_cast<gate_kinds_t>(kind_);
	}

	bool is(gate_kinds_t kind) const
	{
		return kind_ == static_cast<uint32_t>(kind);
	}

	bool is_dependent(mcmt_gate const& other) const
	{
		return ((controls_ & other.targets_) != 0);
	}

	auto is_control(uint32_t qubit_id) const
	{
		return (controls_ & (1u << qubit_id));
	}

	auto rotation_angle() const
	{
		return rotation_angle_;
	}

	auto qubit_index(uint32_t qubit_id) const
	{
		return qubit_id;
	}
#pragma endregion

#pragma region Iterators
	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		for (auto i = targets_, qid = 0u; i; i >>= 1, ++qid) {
			if (i & 1) {
				fn(qid);
			}
		}
	}

	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		for (auto i = controls_, qid = 0u; i; i >>= 1, ++qid) {
			if (i & 1) {
				fn(qid);
			}
		}
	}
#pragma endregion

private:
	uint32_t kind_;
	uint32_t controls_;
	uint32_t targets_;
	float rotation_angle_;
};

} // namespace tweedledum
