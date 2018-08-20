/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "gate_kinds.hpp"

#include <array>
#include <limits>

namespace tweedledum {

class qc_gate {
	static constexpr uint32_t no_qubit = std::numeric_limits<uint16_t>::max();

public:
	static constexpr uint32_t max_num_qubits = 3;

public:
	qc_gate() = default;

	qc_gate(gate_kinds_t kind, uint32_t q0,
	        uint32_t q1 = no_qubit, uint32_t q2 = no_qubit)
	    : kind_(static_cast<uint32_t>(kind))
	    , target_(0)
	    , qubit0_(q0)
	    , qubit1_(q1)
	    , qubit2_(q2)
	{}

	qc_gate(gate_kinds_t kind, uint32_t q0, float rotation)
	    : kind_(static_cast<uint32_t>(kind))
	    , target_(0)
	    , qubit0_(q0)
	    , rotation_(rotation)
	{}

	void kind(gate_kinds_t kind)
	{
		kind_ = static_cast<uint32_t>(kind);
	}

	void target_qubit(uint32_t id)
	{
		switch (target_) {
		case 1:
			qubit1_ = id;
		case 2:
			qubit2_ = id;
		default:
			qubit0_ = id;
		}
	}

	void control(uint32_t id)
	{
		qubit1_ = id;
	}

	float angle() const
	{
		return rotation_;
	}

	gate_kinds_t kind() const
	{
		return static_cast<gate_kinds_t>(kind_);
	}

	uint32_t target() const
	{
		switch (target_) {
		case 1:
			return qubit1_;
		case 2:
			return qubit2_;
		default:
			return qubit0_;
		}
	}

	std::array<uint32_t, 2> controls() const
	{
		if (is_one_of(gate_kinds_t::rotation_x, gate_kinds_t::rotation_z)) {
			return {no_qubit, no_qubit};
		}
		switch (target_) {
		case 1:
			return {qubit0_, qubit2_};
		case 2:
			return {qubit0_, qubit1_};
		default:
			return {qubit1_, qubit2_};
		}
	}

	uint32_t get_input_id(uint32_t qubit_id) const
	{
		if (qubit_id == qubit0_) {
			return 0;
		}
		if (qubit_id == qubit1_) {
			return 1;
		}
		return 2;
	}

	bool is_control(uint32_t qubit_id) const
	{
		if (qubit_id == 0) {
			return false;
		}
		return true;
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

	bool operator==(qc_gate const& other) const
	{
		return data0_ == other.data0_ && data1_ == other.data1_;
	}

	// Assume this function is only called when gates are related in a DAG
	bool is_dependent(qc_gate const& other) const
	{
		if (*this == other) {
			return false;
		}
		if (this->is_z_rotation()) {
			if (other.is_z_rotation()) {
				return false;
			}
			if (other.is_x_rotation()) {
				auto this_controls = other.controls();
				if (this_controls[0] == other.target() || this_controls[1] == other.target()) {
					return true;
				}
				return target() == other.target();
			}
		}
		if (this->is_x_rotation()) {
			auto other_controls = other.controls();
			if (other.is_z_rotation()) {
				if (other_controls[0] == target() || other_controls[1] == target()) {
					return true;
				}
				return target() == other.target();
			}
			if (other.is_x_rotation()) {
				if (target() == other.target()) {
					return false;
				}
				if (other_controls[0] == target() || other_controls[1] == target()) {
					return true;
				}
				auto this_controls = controls();
				if (this_controls[0] == other.target() || this_controls[1] == other.target()) {
					return true;
				}
				return false;
			}
		}
		return true;
	}

	bool is_z_rotation() const
	{
		return is_one_of(gate_kinds_t::phase, gate_kinds_t::phase_dagger,
		                 gate_kinds_t::t, gate_kinds_t::t_dagger,
		                 gate_kinds_t::pauli_z, gate_kinds_t::rotation_z,
				 gate_kinds_t::cz, gate_kinds_t::mcz);
	}

	bool is_x_rotation() const
	{
		return is_one_of(gate_kinds_t::pauli_x, gate_kinds_t::rotation_x,
		                 gate_kinds_t::cx, gate_kinds_t::mcx);
	}

	// This might get me in trouble in the future if someone changes the gate_kinds_t
	bool is_controlled() const
	{
		return kind_ >= static_cast<uint32_t>(gate_kinds_t::cx);
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		switch (target_) {
		case 0:
			fn(qubit0_);
			break;
		case 1:
			fn(qubit1_);
			break;
		case 2:
			fn(qubit2_);
			break;
		default:
			break;
		}
	}

	uint32_t num_controls() const
	{
		if (kind_ < static_cast<uint32_t>(gate_kinds_t::cx)) {
			return 0;
		}

		uint32_t cnt{1};
		if (target_ == 2 || this->is_one_of(gate_kinds_t::mcx, gate_kinds_t::mcz))
		{
			cnt++;
		}
		return cnt;
	}

	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		if (kind_ < static_cast<uint32_t>(gate_kinds_t::cx)) {
			return;
		}
		switch (target_) {
		case 0:
			fn(qubit1_);
			if (this->is_one_of(gate_kinds_t::mcx, gate_kinds_t::mcz)) {
				fn(qubit2_);
			}
			break;
		case 1:
			fn(qubit0_);
			if (this->is_one_of(gate_kinds_t::mcx, gate_kinds_t::mcz)) {
				fn(qubit2_);
			}
			break;
		case 2:
			fn(qubit0_);
			fn(qubit1_);
			break;
		default:
			break;
		}
	}

private:
	union {
		uint32_t data0_;
		struct { 
			uint32_t kind_ : 14;
			uint32_t target_ : 2;
			uint32_t qubit0_ : 16;
		};
	};
	union {
		uint32_t data1_;
		float rotation_;
		struct {
			uint32_t qubit1_ : 16;
			uint32_t qubit2_ : 16;
		};
	};
};

} // namespace tweedledum
