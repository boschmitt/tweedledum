/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/wire.hpp"
#include "../gates/gate.hpp"

#include <array>
#include <cassert>
#include <utility>
#include <vector>

namespace tweedledum {

/*! \brief Three-wire operation */
class w2_op : public gate {
	using gate_type = tweedledum::gate;

#pragma region Helper functions (Init)
	void init_one_io(wire::id const t)
	{
		assert(t.is_qubit() && t != wire::invalid_id && !t.is_complemented());
		assert(is_one_qubit());
		wires_ = {t, wire::invalid_id};
	}

	// When dealing with controlled gates (e.g.CX, CZ) id0 is the control and id1 the target
	// *) In case of SWAP they are both targets
	// *) In case of MEASUREMENT they are both targets and id1 _must_ be the cbit
	void init_two_io(wire::id const w0, wire::id const w1)
	{
		assert(w0.is_qubit() && w0 != wire::invalid_id);
		assert(w1 != wire::invalid_id);
		assert(w0 != w1 && "The wires must be different");
		assert(is_two_qubit() || is_measurement());
		// In a measurement gate the second I/O must be a cbit:
		assert((is_measurement() && !w1.is_qubit()) || !is_measurement());

		wires_ = {w0, w1};
		if (is_measurement()) {
			assert(!w0.is_complemented());
			assert(!w1.is_qubit());
			num_controls_ = 0;
			num_targets_ = 2;
			return;
		}

		// If we reach this point, then w1 is guaranteed to be a target qubit, so it cannot
		// be complemented!
		assert(w1.is_qubit() && !w1.is_complemented());
		if (is(gate_ids::swap)) {
			assert(!w0.is_complemented());
			num_controls_ = 0;
			num_targets_ = 2;
			// Normalization step to make SWAP(0, 1) == SWAP(1, 0);
			if (w1.uid() < w0.uid()) {
				std::swap(wires_.at(0), wires_.at(1));
			}
		}
	}
#pragma endregion

public:
#pragma region Constants
	constexpr static uint32_t max_num_wires = 2;
#pragma endregion

#pragma region Constructors
	w2_op(gate const& g, wire::id const t)
	    : gate(g)
	    , num_controls_(0)
	    , num_targets_(1)
	    , wires_({t.wire(), wire::invalid_id})
	{
		assert(t != wire::invalid_id && !t.is_complemented());
		assert(is_meta() || (is_one_qubit() && t.is_qubit()));
	}

	// When dealing with controlled gates (e.g. CX) id0 is the control and id1 the target
	// *) In case of SWAP they are both targets
	// *) In case of MEASUREMENT they are both targets and id1 _must_ be the cbit 
	w2_op(gate const& g, wire::id const w0, wire::id const w1)
	    : gate(g)
	    , num_controls_(1u)
	    , num_targets_(1u)
	    , wires_({wire::invalid_id, wire::invalid_id})
	{
		init_two_io(w0, w1);
	}

	w2_op(gate const& g, wire::id const c0, wire::id const c1, wire::id const t)
	    : gate(g)
	    , wires_({wire::invalid_id, wire::invalid_id})
	{
		(void) c0;
		(void) c1;
		(void) t;
		// It should never get here
		std::abort();
	}

	w2_op(gate const& g, std::vector<wire::id> const& cs, std::vector<wire::id> const& ts)
	    : gate(g)
	    , num_controls_(cs.size())
	    , num_targets_(ts.size())
	    , wires_({wire::invalid_id, wire::invalid_id})
	{
		assert(ts.size() >= 1u && "The gate must have at least one target");
		assert(ts.size() <= 2u && "The gate must have at most two target");
		assert(cs.size() + ts.size() > 0u);
		assert(cs.size() + ts.size() <= max_num_wires);

		switch (cs.size()) {
		case 0u:
			if (ts.size() == 1) {
				init_one_io(ts.at(0));
			} else if (ts.size() == 2) {
				init_two_io(ts.at(0), ts.at(1));
			} 
			return;
		
		case 1u:
			init_two_io(cs.at(0), ts.at(0));
			return;

		default:
			break;
		}
		// It should never get here
		std::abort();
	}
#pragma endregion

#pragma region Properties
	uint32_t num_wires() const
	{
		return num_targets() + num_controls();
	}

	uint32_t num_controls() const
	{
		return num_controls_;
	}

	uint32_t num_targets() const
	{
		return num_targets_;
	}

	wire::id control(uint32_t const i = 0u) const
	{
		assert(i < num_controls());
		return wires_.at(i);
	}

	wire::id target(uint32_t const i = 0u) const
	{
		assert(i < num_targets());
		return wires_.at(num_controls() + i);
	}

	uint32_t position(wire::id const w_id) const
	{
		assert(w_id != wire::invalid_id);
		if (wires_.at(0).uid() == w_id.uid()) {
			return 0u;
		} else if (wires_.at(1).uid() == w_id.uid()) {
			return 1u;
		}
		std::abort();
	}

	wire::id wire(uint32_t const position) const
	{
		assert(position < max_num_wires);
		assert(wires_.at(position) != wire::invalid_id);
		return wires_.at(position);
	}

	bool is_adjoint(w2_op const& other) const
	{
		assert(!is_meta() && !other.is_meta());
		assert(!is_measurement() && !other.is_measurement());
		if (_gate().is_adjoint(other) == false) {
			return false;
		}
		if (num_controls() != other.num_controls()) {
			return false;
		}
		if (num_targets() != other.num_targets()) {
			return false;
		}
		return wires_ == other.wires_;
	}

	bool is_dependent(w2_op const& other) const
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

		if (is_two_qubit()|| other.is_two_qubit()) {
			// Check if wires intersect
			bool wires_intersec = false;
			if (wires_.at(0u).uid() == other.wires_.at(0u).uid()) {
				wires_intersec = true;
			} else if (wires_.at(0u).uid() == other.wires_.at(1u).uid()) {
				wires_intersec = true;
			} else if (wires_.at(1u).uid() == other.wires_.at(0u).uid()) {
				wires_intersec = true;
			} else if (wires_.at(1u).uid() == other.wires_.at(1u).uid()) {
				wires_intersec = true;
			}
			if (!wires_intersec) {
				return false;
			}
		} else {
			// Both have one wire
			// If targets are the same, then I just need to worry about the axis
			if (target() == other.target()) {
				return axis() != other.axis();
			}
			return false;
		}
		// If one of the gates is a swap gate, they I need to check if the set qubits
		// intersect. If they do, then the gates are dependent.
		if (is(gate_ids::swap) || other.is(gate_ids::swap)) {
			return true;
		}
		// If targets are the same, then I just need to worry about the axis
		if (target() == other.target()) {
			return axis() != other.axis();
		}

		// Well, targets are not the same
		if (is_one_qubit()) {
			return (axis() != rot_axis::z);
		}
		if (other.is_one_qubit()) {
			return (other.axis() != rot_axis::z);
		}
		if (control().uid() == other.control().uid()) {
			return false;
		}
		return (other.axis() != rot_axis::z) || (axis() != rot_axis::z);
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		if (num_controls()) {
			fn(control(0u));
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		fn(target());
		if (num_targets() == 2u) {
			fn(target(1u));
		}
	}
#pragma endregion

#pragma region Overloads
	bool operator==(w2_op const& other) const
	{
		if (_gate() != other._gate()) {
			return false;
		}
		if (num_controls() != other.num_controls()) {
			return false;
		}
		if (num_targets() != other.num_targets()) {
			return false;
		}
		return wires_ == other.wires_;
	}
#pragma endregion

private:
	gate const& _gate() const
	{
		return static_cast<gate const&>(*this);
	}

private:
	uint32_t num_controls_ : 16;
	uint32_t num_targets_ : 16;
	std::array<wire::id, max_num_wires> wires_;
};


} // namespace tweedledum
