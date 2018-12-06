/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/qubit.hpp"
#include "gate_set.hpp"
#include "gate_base.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace tweedledum {

/*! \brief Multiple Control Single Target quantum gate.
 *
 * This class represents a gate which can act upon one, two, or three qubits of a quantum network.
 * Qubit identifier(s) 'qid' indicate on which qubit(s) the gate is acting. One qubit must be the
 * target.
 */
class mcst_gate final : public gate_base {
public:
#pragma region Constants
	constexpr static auto max_num_qubits = 3u;
#pragma endregion

#pragma region Constructors
	mcst_gate(gate_base const& op, qubit_id target)
	    : gate_base(op)
	    , target_(0)
	    , qid_slots_({target, qid_invalid, qid_invalid})
	{
		assert(is_single_qubit());
	}

	mcst_gate(gate_base const& op, qubit_id control, qubit_id target)
	    : gate_base(op)
	    , target_(0)
	    , qid_slots_({target, control, qid_invalid})
	{
		assert(is_double_qubit());
		assert(control != target);
		if (control < target) {
			std::swap(qid_slots_[0], qid_slots_[1]);
			target_ = 1;
		}
	}

	mcst_gate(gate_base const& op, std::vector<qubit_id> const& controls,
	          std::vector<qubit_id> const& target)
	    : gate_base(op)
	    , target_(0)
	    , qid_slots_({qid_invalid, qid_invalid, qid_invalid})
	{
		assert(controls.size() == num_controls());
		assert(target.size() == 1u);
		for (auto i = 0u; i < controls.size(); ++i) {
			qid_slots_[i] = controls[i];
		}
		qid_slots_[2] = target[0];
		std::sort(qid_slots_.begin(), qid_slots_.end());
		for (auto i = 0u; i < qid_slots_.size(); ++i) {
			if (qid_slots_[i] == target[0]) {
				target_ = i;
				break;
			}
		}
	}
#pragma endregion

#pragma region Properties
	uint32_t num_controls() const
	{
		assert(!is_meta());
		if (is_single_qubit()) {
			return 0u;
		}
		auto num_controls = 1u;
		if (is_one_of(gate_set::mcx, gate_set::mcz)) {
			++num_controls;
		}
		return num_controls;
	}

	constexpr auto num_targets() const
	{
		assert(!is_meta());
		return 1u;
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		assert(!is_meta());
		if (is_single_qubit()) {
			return;
		}
		for (auto slot = 0u; slot < max_num_qubits; ++slot) {
			if (slot == target_ || qid_slots_[slot] == qid_invalid) {
				continue;
			}
			if constexpr (std::is_invocable_r<bool, Fn, qubit_id>::value) {
				if (!fn(qid_slots_[slot])) {
					return;
				}
			} else {
				fn(qid_slots_[slot]);
			}
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		assert(!is_meta());
		fn(qid_slots_[target_]);
	}
#pragma endregion

private:
	/*! \brief indicates the slot which holds the qid of the target. */
	uint8_t target_;
	/*! \brief an array which hold the qids' of the qubits this gate is acting upon */
	std::array<qubit_id, max_num_qubits> qid_slots_;
};

} // namespace tweedledum
