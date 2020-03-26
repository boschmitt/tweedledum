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
class w3_op : public gate {
	using gate_type = tweedledum::gate;

#pragma region Helper functions (Init)
	void init_one_io(wire_id target)
	{
		assert(target.is_qubit() && target != wire::invalid && !target.is_complemented());
		assert(is_one_qubit());
		wires_ = {target, wire::invalid, wire::invalid};
	}

	// When dealing with controlled gates (e.g.CX, CZ) id0 is the control and id1 the target
	// *) In case of SWAP they are both targets
	// *) In case of MEASUREMENT they are both targets and id1 _must_ be the cbit
	void init_two_io(wire_id w0, wire_id w1)
	{
		assert(w0.is_qubit() && w0 != wire::invalid);
		assert(w1 != wire::invalid);
		assert(w0 != w1 && "The wires must be different");
		assert(is_two_qubit() || is_measurement());
		// In a measurement gate the second I/O must be a cbit:
		assert((is_measurement() && !w1.is_qubit()) || !is_measurement());

		wires_ = {w0, w1, wire::invalid};
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
			if (w1.id() < w0.id()) {
				std::swap(wires_.at(0), wires_.at(1));
			}
		}
	}

	void init_three_io(wire_id control0, wire_id control1, wire_id target)
	{
		assert(control0.is_qubit() && control0 != wire::invalid);
		assert(control1.is_qubit() && control1 != wire::invalid);
		assert(target.is_qubit() && target != wire::invalid && !target.is_complemented());
		assert(control0 != control1 && control0 != target && control1 != target);
		assert(!is_meta() && !is_measurement());
		assert(!is_one_qubit() && !is_two_qubit());

		wires_ = {control0, control1, target};
		// Normalization step to make CCX(0, 1, 2) == CCX(1, 0, 2);
		if (control1.id() < control0.id()) {
			std::swap(wires_.at(0), wires_.at(1));
		}
	}
#pragma endregion

public:
#pragma region Constants
	constexpr static uint32_t max_num_wires = 3u;
#pragma endregion

#pragma region Constructors
	w3_op(gate const& g, wire_id target)
	    : gate(g)
	    , num_controls_(0)
	    , num_targets_(1)
	    , wires_({target.wire(), wire::invalid, wire::invalid})
	{
		assert(target != wire::invalid && !target.is_complemented());
		assert(is_meta() || (is_one_qubit() && target.is_qubit()));
	}

	// When dealing with controlled gates (e.g. CX) id0 is the control and id1 the target
	// *) In case of SWAP they are both targets
	// *) In case of MEASUREMENT they are both targets and id1 _must_ be the cbit 
	w3_op(gate const& g, wire_id w0, wire_id w1)
	    : gate(g)
	    , num_controls_(1u)
	    , num_targets_(1u)
	    , wires_({wire::invalid, wire::invalid, wire::invalid})
	{
		init_two_io(w0, w1);
	}

	w3_op(gate const& g, wire_id control0, wire_id control1, wire_id target)
	    : gate(g)
	    , num_controls_(2u)
	    , num_targets_(1u)
	    , wires_({wire::invalid, wire::invalid, wire::invalid})
	{
		init_three_io(control0, control1, target.wire());
	}

	w3_op(gate const& g, std::vector<wire_id> const& controls, std::vector<wire_id> const& targets)
	    : gate(g)
	    , num_controls_(controls.size())
	    , num_targets_(targets.size())
	    , wires_({wire::invalid, wire::invalid, wire::invalid})
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

		case 2u:
			init_three_io(controls.at(0), controls.at(1), targets.at(0));
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
		} else if (wires_.at(2).id() == wire.id()) {
			return 2u;
		}
		std::abort();
	}

	uint32_t wire(uint32_t position) const
	{
		assert(position < max_num_wires);
		assert(wires_.at(position) != wire::invalid);
		return wires_.at(position);
	}

	bool is_adjoint(w3_op const& other) const
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

	bool is_dependent(w3_op const& other) const
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
		// If one of the gates is a swap gate, they I need to check if the set qubits
		// intersect. If they do, then the gates are dependent.
		if (is(gate_ids::swap) || other.is(gate_ids::swap)) {
			for (uint32_t i = 0; i < wires_.size(); ++i) {
				if (wires_.at(i) == wire::invalid) {
					break;
				}
				for (uint32_t j = 0; j < other.wires_.size(); ++j) {
					if (wires_.at(i) == other.wires_.at(0)) {
						return true;
					}
				}
			}
			return false;
		}
		// If targets are the same, then I just need to worry about the axis
		if (target() == other.target()) {
			return axis() != other.axis();
		}

		// Well, targets are not the same, and both are an one-qubit operation:
		if (is_one_qubit() && other.is_one_qubit()) {
			return false;
		}

		if (axis() == rot_axis::z) {
			if (other.axis() == rot_axis::z) {
				return false;
			}
			bool dependent = false;
			foreach_control([&](wire_id this_control) {
				other.foreach_target([&](wire_id other_target) {
					dependent |= (this_control.id() == other_target);
				});
			});
			return dependent;
		}
		if (other.axis() == rot_axis::z) {
			bool dependent = false;
			other.foreach_control([&](wire_id other_control) {
				dependent |= (other_control.id() == target());
			});
			return dependent;
		}

		bool dependent = false;
		foreach_control([&](wire_id this_control) {
			dependent |= (this_control.id() == other.target());
		});
		other.foreach_control([&](wire_id other_control) {
			dependent |= (target() == other_control.id());
		});
		return dependent;
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		switch (num_controls()) {
		case 1u:
			fn(control(0u));
			return;
			
		case 2u:
			fn(control(0u));
			fn(control(1u));
			return;
		
		default:
			break;
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
	bool operator==(w3_op const& other) const
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
	std::array<wire_id, max_num_wires> wires_;
};


} // namespace tweedledum
