/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/wire_id.hpp"
#include "../gates/gate.hpp"

#include <array>
#include <cassert>
#include <utility>
#include <vector>

namespace tweedledum {

/*! \brief Three-wire operation */
class w2_op {
	using gate_type = tweedledum::gate;

#pragma region Helper functions (Init)
	void init_one_io(wire_id target)
	{
		assert(target.is_qubit() && target != wire::invalid && !target.is_complemented());
		assert(gate.is_one_qubit());
		wires_ = {target, wire::invalid};
	}

	// When dealing with controlled gates (e.g.CX, CZ) id0 is the control and id1 the target
	// *) In case of SWAP they are both targets
	// *) In case of MEASUREMENT they are both targets and id1 _must_ be the cbit
	void init_two_io(wire_id w0, wire_id w1)
	{
		assert(w0.is_qubit() && w0 != wire::invalid);
		assert(w1 != wire::invalid);
		assert(w0 != w1 && "The wires must be different");
		assert(gate.is_two_qubit() || gate.is_measurement());
		// In a measurement gate the second I/O must be a cbit:
		assert((gate.is_measurement() && !w1.is_qubit()) || !gate.is_measurement());

		wires_ = {w0, w1};
		if (gate.is_measurement()) {
			assert(!w0.is_complemented());
			assert(!w1.is_qubit());
			num_controls_ = 0;
			num_targets_ = 2;
			return;
		}

		// If we reach this point, then w1 is guaranteed to be a target qubit, so it cannot
		// be complemented!
		assert(w1.is_qubit() && !w1.is_complemented());
		if (gate.is(gate_ids::swap)) {
			assert(!w0.is_complemented());
			num_controls_ = 0;
			num_targets_ = 2;
			// Normalization step to make SWAP(0, 1) == SWAP(1, 0);
			if (w1.id() < w0.id()) {
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
	w2_op(gate const& g, wire_id target)
	    : gate(g)
	    , num_controls_(0)
	    , num_targets_(1)
	    , wires_({target.wire(), wire::invalid})
	{
		assert(target != wire::invalid && !target.is_complemented());
		assert(gate.is_meta() || (gate.is_one_qubit() && target.is_qubit()));
	}

	// When dealing with controlled gates (e.g. CX) id0 is the control and id1 the target
	// *) In case of SWAP they are both targets
	// *) In case of MEASUREMENT they are both targets and id1 _must_ be the cbit 
	w2_op(gate const& g, wire_id w0, wire_id w1)
	    : gate(g)
	    , num_controls_(1u)
	    , num_targets_(1u)
	    , wires_({wire::invalid, wire::invalid})
	{
		init_two_io(w0, w1);
	}

	w2_op(gate const& g, wire_id control0, wire_id control1, wire_id target)
	    : gate(g)
	    , wires_({wire::invalid, wire::invalid})
	{
		(void) control0;
		(void) control1;
		(void) target;
		// It should never get here
		std::abort();
	}

	w2_op(gate const& g, std::vector<wire_id> const& controls, std::vector<wire_id> const& targets)
	    : gate(g)
	    , num_controls_(controls.size())
	    , num_targets_(targets.size())
	    , wires_({wire::invalid, wire::invalid})
	{
		assert(targets.size() >= 1u && "The gate must have at least one target");
		assert(targets.size() <= 2u && "The gate must have at most two target");
		assert(controls.size() + targets.size() > 0u);
		assert(controls.size() + targets.size() <= max_num_wires);

		switch (controls.size()) {
		case 0u:
			if (targets.size() == 1) {
				init_one_io(targets.at(0));
			} else if (targets.size() == 2) {
				init_two_io(targets.at(0), targets.at(1));
			} 
			return;
		
		case 1u:
			init_two_io(controls.at(0), targets.at(0));
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

	wire_id control(uint32_t i = 0u) const
	{
		assert(i < num_controls());
		return wires_.at(i);
	}

	wire_id target(uint32_t i = 0u) const
	{
		assert(i < num_targets());
		return wires_.at(num_controls() + i);
	}

	uint32_t position(wire_id wire) const
	{
		assert(wire != wire::invalid);
		if (wires_.at(0).id() == wire.id()) {
			return 0u;
		} else if (wires_.at(1).id() == wire.id()) {
			return 1u;
		}
		std::abort();
	}

	uint32_t wire(uint32_t position) const
	{
		assert(position < max_num_wires);
		assert(wires_.at(position) != wire::invalid);
		return wires_.at(position);
	}

	bool is_adjoint(w2_op const& other) const
	{
		assert(!gate.is_meta() && !other.gate.is_meta());
		assert(!gate.is_measurement() && !other.gate.is_measurement());
		if (gate.is_adjoint(other.gate) == false) {
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
		if (gate.is_meta() || other.gate.is_meta()) {
			return true;
		}
		// When the gates are equal, they are _not_ dependent.
		if (*this == other) {
			return false;
		}
		// If one of the gates is an identity gate, they are _not_ dependent.
		if (gate.is(gate_ids::i) || other.gate.is(gate_ids::i)) {
			return false;
		}

		if (gate.is_two_qubit()|| other.gate.is_two_qubit()) {
			// Check if wires intersect
			bool wires_intersec = false;
			if (wires_.at(0u).id() == other.wires_.at(0u).id()) {
				wires_intersec = true;
			} else if (wires_.at(0u).id() == other.wires_.at(1u).id()) {
				wires_intersec = true;
			} else if (wires_.at(1u).id() == other.wires_.at(0u).id()) {
				wires_intersec = true;
			} else if (wires_.at(1u).id() == other.wires_.at(1u).id()) {
				wires_intersec = true;
			}
			if (!wires_intersec) {
				return false;
			}
		} else {
			// Both have one wire
			// If targets are the same, then I just need to worry about the axis
			if (target() == other.target()) {
				return gate.axis() != other.gate.axis();
			}
			return false;
		}
		// If one of the gates is a swap gate, they I need to check if the set qubits
		// intersect. If they do, then the gates are dependent.
		if (gate.is(gate_ids::swap) || other.gate.is(gate_ids::swap)) {
			return true;
		}
		// If targets are the same, then I just need to worry about the axis
		if (target() == other.target()) {
			return gate.axis() != other.gate.axis();
		}

		// Well, targets are not the same
		if (gate.is_one_qubit()) {
			return (gate.axis() != rot_axis::z);
		}
		if (other.gate.is_one_qubit()) {
			return (other.gate.axis() != rot_axis::z);
		}
		if (control().id() == other.control().id()) {
			return false;
		}
		return (other.gate.axis() != rot_axis::z) || (gate.axis() != rot_axis::z);
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
		if (gate != other.gate) {
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

public:
	gate const gate;

private:
	uint32_t num_controls_ : 16;
	uint32_t num_targets_ : 16;
	std::array<wire_id, max_num_wires> wires_;
};


} // namespace tweedledum
