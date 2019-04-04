/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/io_id.hpp"
#include "gate_base.hpp"
#include "gate_set.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace tweedledum {

/*! \brief Quantum gate
 *
 * This class represents a gate which can act upon one, two, or three qubits of a quantum network.
 * Qubit identifier(s) 'qid' indicate on which qubit(s) the gate is acting. At least one qubit must
 * be the target, but, in case of a SWAP, the gate will have two targets.
 */
class q_gate final : public gate_base {
public:
#pragma region Constants
	constexpr static auto max_num_io = 3u;
#pragma endregion

#pragma region Constructors
	q_gate(gate_base const& op, io_id target)
	    : gate_base(op)
	    , target0_(0)
	    , target1_(std::numeric_limits<uint8_t>::max())
	    , control0_(std::numeric_limits<uint8_t>::max())
	    , control1_(std::numeric_limits<uint8_t>::max())
	    , qids_({target, io_invalid, io_invalid})
	{
		assert(is_single_qubit());
	}

	// When dealing with CX and CZ q0 is the control and q1 the target
	// in case of swaps they are both targets
	q_gate(gate_base const& op, io_id q0, io_id q1)
	    : gate_base(op)
	    , target0_(0)
	    , target1_(std::numeric_limits<uint8_t>::max())
	    , control0_(1)
	    , control1_(std::numeric_limits<uint8_t>::max())
	    , qids_({q1, q0, io_invalid})
	{
		assert(is_double_qubit());
		assert(q0 != q1); 
		if (q0 < q1) {
			std::swap(qids_[0], qids_[1]);
			std::swap(target0_, control0_);
		}
		if (op.is(gate_set::swap)) {
			std::swap(target1_, control0_);
		}
	}

	q_gate(gate_base const& op, std::vector<io_id> const& controls,
	       std::vector<io_id> const& target)
	    : gate_base(op)
	    , target0_(0)
	    , target1_(std::numeric_limits<uint8_t>::max())
	    , control0_(std::numeric_limits<uint8_t>::max())
	    , control1_(std::numeric_limits<uint8_t>::max())
	    , qids_({io_invalid, io_invalid, io_invalid})
	{
		assert(controls.size() == num_controls());
		assert(target.size() == 1u);
		for (auto i = 0u; i < controls.size(); ++i) {
			qids_[i] = controls[i];
		}
		qids_[2] = target[0];
		std::sort(qids_.begin(), qids_.end());
		target0_ = std::distance(qids_.begin(), std::find(qids_.begin(), qids_.end(), target[0]));
		switch (target0_) {
			case 0u:
				control0_ = 1;
				control1_ = 2;
				break;

			case 1u:
				control0_ = 0;
				control1_ = 2;
				break;

			case 2u:
				control0_ = 0;
				control1_ = 1;
				break;

			default:
				assert(0);
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

	uint32_t num_targets() const
	{
		if (is(gate_set::swap)) {
			return 2u;
		}
		return 1u;
	}

	io_id target() const
	{
		if (!is(gate_set::swap)) {
			return io_invalid;
		}
		return qid_slots_[target0_];
	}

	io_id control() const
	{
		if (!is_one_of(gate_set::cx, gate_set::cz)) {
			return io_invalid;
		}
		return qid_slots_[control0_];
	}

	bool is_control(io_id qid) const
	{
		return (qids_[control0_] == qid || qids_[control1_] == qid);
	}

	uint32_t qubit_slot(io_id qid) const
	{
		for (auto i = 0u; i < qids_.size(); ++i) {
			if (qids_[i].index() == qid.index()) {
				return i;
			}
		}
		// This is unreachable
		assert(0);
		std::abort();
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		if (control0_ != std::numeric_limits<uint8_t>::max()) {
			fn(qids_[control0_]);
		}
		if (control1_ != std::numeric_limits<uint8_t>::max()) {
			fn(qids_[control1_]);
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		fn(qids_[target0_]);
		if (target1_ != std::numeric_limits<uint8_t>::max()) {
			fn(qids_[target1_]);
		}
	}
#pragma endregion

private:
	/*! \brief indicates the slot which holds the qid of the target. */
	uint8_t target0_;
	uint8_t target1_;
	uint8_t control0_;
	uint8_t control1_;
	/*! \brief an array which hold the qids' of the qubits this gate is acting upon */
	std::array<io_id, max_num_io> qids_;
};

} // namespace tweedledum
