/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/io_id.hpp"
#include "gate_base.hpp"
#include "gate_lib.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace tweedledum {

/*! \brief Multiple Control Multiple Target reversible gate
 *
 * This class represents a gate which can act upon up to 32 qubits of a quantum network. This gate
 * type needs the network itself to have a maximum number o qubits of 32 (need to figure out how to
 * do this).
 */
class mcmt_gate final : public gate_base {
public:
#pragma region Constants
	constexpr static auto max_num_io = 32u;
	constexpr static auto network_max_num_io = 32u;
#pragma endregion

#pragma region Constructors
	mcmt_gate(gate_base const& op, io_id target)
	    : gate_base(op)
	    , is_qubit_(0)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(0)
	{
		assert(!is_double_qubit());
		assert(target <= network_max_num_io);
		is_qubit_ |= (target.is_qubit() << target);
		targets_ |= (1 << target);
	}

	mcmt_gate(gate_base const& op, io_id control, io_id target)
	    : gate_base(op)
	    , is_qubit_(0)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(0)
	{
		assert(is_double_qubit());
		assert(control <= network_max_num_io);
		assert(target <= network_max_num_io);
		assert(control != target);

		is_qubit_ |= (control.is_qubit() << control);
		is_qubit_ |= (target.is_qubit() << target);
		targets_ |= (1 << target);
		if (is(gate_lib::swap)) {
			targets_ |= (1 << control);
			assert(num_targets() == 2 && "Swap gates can only have two targets.");
		} else {
			controls_ |= (1 << control);
			polarity_ |= (control.is_complemented() << control.index());
		}
	}

	mcmt_gate(gate_base const& op, std::vector<io_id> const& controls,
	          std::vector<io_id> const& targets)
	    : gate_base(op)
	    , is_qubit_(0)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(0)
	{
		assert(controls.size() <= max_num_io);
		assert(targets.size() > 0 && targets.size() <= max_num_io);
		for (auto control : controls) {
			assert(control <= network_max_num_io);
			controls_ |= (1u << control);
			polarity_ |= (control.is_complemented() << control);
			is_qubit_ |= (control.is_qubit() << control);
		}
		for (auto target : targets) {
			assert(target <= network_max_num_io);
			targets_ |= (1u << target);
			is_qubit_ |= (target.is_qubit() << target);
		}
		assert((targets_ & controls_) == 0u);
	}
#pragma endregion

#pragma region Properties
	uint32_t num_controls() const
	{
		return __builtin_popcount(controls_);
	}

	uint32_t num_targets() const
	{
		return __builtin_popcount(targets_);
	}

	uint32_t num_io() const
	{
		return num_targets() + num_controls();
	}

	io_id target() const
	{
		if (num_targets() > 1) {
			return io_invalid;
		}
		const uint32_t idx = __builtin_ctz(targets_);
		return io_id(idx, (is_qubit_ >> idx) & 1);
	}

	io_id control() const
	{
		if (!is_one_of(gate_lib::cx, gate_lib::cz)) {
			return io_invalid;
		}
		const uint32_t idx = __builtin_ctz(controls_);
		return io_id(idx, (is_qubit_ >> idx) & 1, (polarity_ >> idx) & 1);
	}

	bool is_control(io_id qid) const
	{
		return (controls_ & (1u << qid.index()));
	}

	uint32_t qubit_slot(io_id qid) const
	{
		return qid.index();
	}

	io_id qubit(uint32_t slot) const
	{
		assert(slot < max_num_io);
		if ((1u << slot) & (controls_ | targets_)) {
			return io_id(slot, (polarity_ >> slot) & 1);
		}
		return io_invalid;
	}

	bool is_adjoint(mcmt_gate const& other) const
	{
		if (this->is_op_adjoint(other) == false) {
			return false;
		}
		bool not_adj = false;
		not_adj |= (controls_ != other.controls_);
		not_adj |= (polarity_ != other.polarity_);
		not_adj |= (targets_ != other.targets_);
		return !not_adj;
	}

	bool is_dependent(mcmt_gate const& other) const
	{
		if (this->is_meta() || other.is_meta()) {
			return true;
		}
		// The easiest case is when the gates are equal, then they are _not_ dependent.
		if (*this == other) {
			return false;
		}
		// Checking dependency for Z-axis rotation is 'weird'.  Basically these rotations
		// do not interfere with controls of other gates, hence if `this` is a Z-axis
		// rotation, I just need to guarantee that `this.controls` and `this.targets` do
		// _not_ intersect with the `other.targets` do when other is _not_ a Z-axis
		// rotation.  Otherwise, the gates are independent, i.e. both are Z-axis rotation.
		//
		// If `other` is a Z-axis rotation then the gates are _not_ dependent.
		if (this->is_z_rotation()) {
			if (other.is_z_rotation()) {
				return false;
			}
			return (((controls_ | targets_) & other.targets_) != 0);
		}
		// If the other gate is a Z-axis rotation then the gate will be dependet as long
		// as the intersection between the targets is _not_ empty, i.e. 0.
		if (other.is_z_rotation()) {
			return ((targets_ & other.targets_) != 0);
		}
		// Both gates are _not_ Z-axis rotation, then they will be dependet as long as the
		// intersection between `this.targets` and `other.controls_` is _not_ empty, and 
		// vice versa.
		if ((targets_ & other.controls_) != 0) {
			return true;
		} else if ((controls_ & other.targets_) != 0) {
			return true;
		}
		// Finaly, they will be dependent as long as their rotation axis are diferent.
		return this->rotation_axis() != other.rotation_axis();
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		uint32_t c = controls_;
		uint32_t q = is_qubit_;
		uint32_t p = polarity_;
		for (uint32_t idx = 0u; c; c >>= 1, q >>= 1, p >>= 1, ++idx) {
			if (c & 1) {
				fn(io_id(idx, (q & 1), (p & 1)));
			}
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		uint32_t t = targets_;
		uint32_t q = is_qubit_;
		for (uint32_t idx = 0u; t; t >>= 1, q >>= 1, ++idx) {
			if (t & 1) {
				fn(io_id(idx, (q & 1)));
			}
		}
	}
#pragma endregion

#pragma region Overloads
	bool operator==(mcmt_gate const& other) const
	{
		if (operation() != other.operation()) {
			return false;
		}
		bool equal = true;
		equal &= (is_qubit_ == other.is_qubit_);
		equal &= (polarity_ == other.polarity_);
		equal &= (controls_ == other.controls_);
		equal &= (targets_ == other.targets_);
		return equal;
	}
#pragma endregion

private:
	/*! \brief bitmap which indicates which i/o in the network are the qubits. */
	uint32_t is_qubit_;
	/*! \brief bitmap which indicates the controls' polarities. */
	uint32_t polarity_;
	/*! \brief bitmap which indicates which i/o in the network are the controls. */
	uint32_t controls_;
	/*! \brief bitmap which indicates which i/o in the network are the targets. */
	uint32_t targets_;
};

} // namespace tweedledum
