/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/wire.hpp"
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
	wn32_op(gate const& g, wire::id const t)
	    : gate(g)
	    , is_qubit_(t.is_qubit() << t)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(1u << t)
	{
		assert(t!= wire::invalid_id && !t.is_complemented());
		assert(is_meta() || (is_one_qubit() && t.is_qubit()));
	}

	wn32_op(gate const& g, wire::id const w0, wire::id const w1)
	    : gate(g)
	    , is_qubit_((w0.is_qubit() << w0) | (w1.is_qubit() << w1))
	    , polarity_(w0.is_complemented() << w0)
	    , controls_(1u << w0)
	    , targets_(1u << w1)
	{
		assert(w0 != wire::invalid_id);
		assert(w1 != wire::invalid_id);
		assert(w0.uid() != w1.uid());

		if (is(gate_ids::swap)) {
			targets_ |= (1 << w0);
			polarity_ = 0u;
			controls_ = 0u;
		}
	}

	wn32_op(gate const& g, wire::id const c0, wire::id const c1, wire::id const t)
	    : gate(g)
	    , is_qubit_((c0.is_qubit() << c0) | (c1.is_qubit() << c1) | (t.is_qubit() << t))
	    , polarity_((c0.is_complemented() << c0) | (c1.is_complemented() << c1))
	    , controls_((1u << c0) | (1u << c1))
	    , targets_(1u << t)
	{
		assert(c0 != wire::invalid_id);
		assert(c1 != wire::invalid_id);
		assert(t != wire::invalid_id);
		assert(c0.uid() != c1.uid() && c0.uid() != t.uid() && c1.uid() != t.uid());
		assert(!is_meta() && !is_measurement());
		assert(!is_one_qubit() && !is_two_qubit());
	}

	wn32_op(gate const& g, std::vector<wire::id> const& cs, std::vector<wire::id> const& ts)
	    : gate(g)
	    , is_qubit_(0)
	    , polarity_(0)
	    , controls_(0)
	    , targets_(0)
	{
		assert(!ts.empty());
		assert(cs.size() + ts.size() <= max_num_wires);
		for (wire::id control : cs) {
			assert(control <= network_max_num_wires);
			controls_ |= (1u << control);
			polarity_ |= (control.is_complemented() << control);
			is_qubit_ |= (control.is_qubit() << control);
		}
		for (wire::id target : ts) {
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

	wire::id control(uint32_t const i = 0u) const
	{
		assert(i < num_controls());
		uint32_t idx = __builtin_ctz(controls_);
		if (i == 0) {
			return wire::id(idx, (is_qubit_ >> idx) & 1, (polarity_ >> idx) & 1);
		}
		uint32_t temp = (controls_ >> (idx + 1));
		for (uint32_t j = 0; j < i; ++j) {
			uint32_t tmp_idx = __builtin_ctz(temp);
			idx += tmp_idx + 1;
			temp = (controls_ >> (tmp_idx + 1));
		}
		return wire::id(idx, (is_qubit_ >> idx) & 1, (polarity_ >> idx) & 1);
	}

	wire::id target(uint32_t i = 0u) const
	{
		assert(i < num_targets());
		uint32_t idx = __builtin_ctz(targets_);
		if (i == 0) {
			return wire::id(idx, (is_qubit_ >> idx) & 1);
		}
		uint32_t temp = (targets_ >> (idx + 1));
		for (uint32_t j = 0; j < i; ++j) {
			uint32_t tmp_idx = __builtin_ctz(temp);
			idx += tmp_idx + 1;
			temp = (targets_ >> (tmp_idx + 1));
		}
		return wire::id(idx, (is_qubit_ >> idx) & 1);
	}

	uint32_t position(wire::id const w_id) const
	{
		assert(w_id != wire::invalid_id);
		return w_id.uid();
	}

	wire::id wire(uint32_t const position) const
	{
		assert(position < max_num_wires);
		if ((1u << position) & (controls_ | targets_)) {
			return wire::id(position, (polarity_ >> position) & 1);
		}
		return wire::invalid_id;
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
				fn(wire::id(idx, (q & 1), (p & 1)));
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
				fn(wire::id(idx, (q & 1)));
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
