/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/wire_id.hpp"
#include "../gates/gate.hpp"

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
 * type needs the network itself to have a maximum number o qubits of 32.
 */
class wn32_op : public gate {
public:
#pragma region Constants
	constexpr static auto max_num_wires = 32u;
	constexpr static auto network_max_num_wires = 32u;
#pragma endregion

#pragma region Constructors
	wn32_op(gate const& g, wire_id target)
	    : gate(g)
	    , is_qubit_(target.is_qubit() << target)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(1u << target)
	{
		assert(target != wire::invalid && !target.is_complemented());
		assert(is_meta() || (is_one_qubit() && target.is_qubit()));
	}

	wn32_op(gate const& g, wire_id w0, wire_id w1)
	    : gate(g)
	    , is_qubit_((w0.is_qubit() << w0) | (w1.is_qubit() << w1))
	    , polarity_(w0.is_complemented() << w0)
	    , controls_(1u << w0)
	    , targets_(1u << w1)
	{
		assert(w0 != wire::invalid);
		assert(w1 != wire::invalid);
		assert(w0.id() != w1.id());

		if (is(gate_ids::swap)) {
			targets_ |= (1 << w0);
			polarity_ = 0u;
			controls_ = 0u;
		}
	}

	wn32_op(gate const& g, wire_id c0, wire_id c1, wire_id target)
	    : gate(g)
	    , is_qubit_((c0.is_qubit() << c0) | (c1.is_qubit() << c1)
	                | (target.is_qubit() << target))
	    , polarity_((c0.is_complemented() << c0) | (c1.is_complemented() << c1))
	    , controls_((1u << c0) | (1u << c1))
	    , targets_(1u << target)
	{
		assert(c0 != wire::invalid);
		assert(c1 != wire::invalid);
		assert(target != wire::invalid);
		assert(c0.id() != c1.id() && c0.id() != target.id() && c1.id() != target.id());
		assert(!is_meta() && !is_measurement());
		assert(!is_one_qubit() && !is_two_qubit());
	}

	wn32_op(gate const& g, std::vector<wire_id> const& controls,
	        std::vector<wire_id> const& targets)
	    : gate(g)
	    , is_qubit_(0)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(0)
	{
		assert(!targets.empty());
		assert(controls.size() + targets.size() <= max_num_wires);
		for (wire_id control : controls) {
			assert(control <= network_max_num_wires);
			controls_ |= (1u << control);
			polarity_ |= (control.is_complemented() << control);
			is_qubit_ |= (control.is_qubit() << control);
		}
		for (wire_id target : targets) {
			assert(target <= network_max_num_wires);
			targets_ |= (1u << target);
			is_qubit_ |= (target.is_qubit() << target);
		}
		assert((targets_ & controls_) == 0u);
	}
#pragma endregion

#pragma region Properties
	uint32_t num_wires() const
	{
		return num_targets() + num_controls();
	}

	uint32_t num_controls() const
	{
		return __builtin_popcount(controls_);
	}

	uint32_t num_targets() const
	{
		return __builtin_popcount(targets_);
	}

	wire_id control(uint32_t i = 0u) const
	{
		assert(i < num_controls());
		uint32_t idx = __builtin_ctz(controls_);
		if (i == 0) {
			return wire_id(idx, (is_qubit_ >> idx) & 1, (polarity_ >> idx) & 1);
		}
		uint32_t temp = (controls_ >> (idx + 1));
		for (uint32_t j = 0; j < i; ++j) {
			uint32_t tmp_idx = __builtin_ctz(temp);
			idx += tmp_idx + 1;
			temp = (controls_ >> (tmp_idx + 1));
		}
		return wire_id(idx, (is_qubit_ >> idx) & 1, (polarity_ >> idx) & 1);
	}

	wire_id target(uint32_t i = 0u) const
	{
		assert(i < num_targets());
		uint32_t idx = __builtin_ctz(targets_);
		if (i == 0) {
			return wire_id(idx, (is_qubit_ >> idx) & 1);
		}
		uint32_t temp = (targets_ >> (idx + 1));
		for (uint32_t j = 0; j < i; ++j) {
			uint32_t tmp_idx = __builtin_ctz(temp);
			idx += tmp_idx + 1;
			temp = (targets_ >> (tmp_idx + 1));
		}
		return wire_id(idx, (is_qubit_ >> idx) & 1);
	}

	uint32_t position(wire_id wire) const
	{
		assert(wire != wire::invalid);
		return wire.id();
	}

	wire_id wire(uint32_t position) const
	{
		assert(position < max_num_wires);
		if ((1u << position) & (controls_ | targets_)) {
			return wire_id(position, (polarity_ >> position) & 1);
		}
		return wire::invalid;
	}

	bool is_adjoint(wn32_op const& other) const
	{
		assert(!is_meta() && !other.is_meta());
		assert(!is_measurement() && !other.is_measurement());
		if (_gate().is_adjoint(other) == false) {
			return false;
		}
		bool not_adj = false;
		not_adj |= (controls_ != other.controls_);
		not_adj |= (polarity_ != other.polarity_);
		not_adj |= (targets_ != other.targets_);
		return !not_adj;
	}

	bool is_dependent(wn32_op const& other) const
	{
		// First, we deal with the easy cases:
		// If one of the gates is a meta gate, then we act conservativaly and return true.
		if (is_meta() || other.is_meta()) {
			return true;
		}
		// When the gates are equal, they are _not_ dependent.
		if (*this == other) {
			return false;
		}
		// If one of the gates is an identity gate, they are _not_ dependent.
		if (is(gate_ids::i) || other.is(gate_ids::i)) {
			return false;
		}
		if (((controls_ | targets_) & (other.controls_ | other.targets_)) == 0u) {
			return false;
		}
		if (is(gate_ids::swap) || other.is(gate_ids::swap)) {
			return true;
		}
		
		uint32_t tt = targets_ & other.targets_;
		uint32_t ct = controls_ & other.targets_;
		uint32_t tc = targets_ & other.controls_;
		if (axis() == rot_axis::z) {
			if (other.axis() == rot_axis::z) {
				return false;
			}
			return (ct | tt);
		}
		if (other.axis() == rot_axis::z) {
			return (tc | tt);
		}
		if ((ct | tc) == 0u) {
			// If have at least one target in common, and their rotation axis are
			// different, then they are dependent:
			return (tt && (axis() != other.axis()));
			// Otherwise they are _not_ dependent
		}
		return true;
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
				fn(wire_id(idx, (q & 1), (p & 1)));
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
				fn(wire_id(idx, (q & 1)));
			}
		}
	}
#pragma endregion

#pragma region Overloads
	bool operator==(wn32_op const& other) const
	{
		if (_gate() != other._gate()) {
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
	gate const& _gate() const
	{
		return static_cast<gate const&>(*this);
	}

private:
	/*! \brief bitmap which indicates which wire in the network are the qubits. */
	uint32_t is_qubit_;
	/*! \brief bitmap which indicates the controls' polarities. */
	uint32_t polarity_;
	/*! \brief bitmap which indicates which wire in the network are the controls. */
	uint32_t controls_;
	/*! \brief bitmap which indicates which wire in the network are the targets. */
	uint32_t targets_;
};

} // namespace tweedledum
