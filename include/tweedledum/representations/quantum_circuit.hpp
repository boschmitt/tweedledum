/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include "gate_kinds.hpp"

#include <iostream>
#include <string>

namespace tweedledum {

class gate {
public:
	static constexpr uint32_t max_num_qubits = 2;

public:
	gate() = default;

	void kind(gate_kinds kind)
	{
		kind_ = static_cast<std::uint8_t>(kind);
	}

	void target(std::uint32_t id)
	{
		target_ = id;
	}

	void control(std::uint32_t id)
	{
		control_ = id;
	}

	gate_kinds kind() const
	{
		return static_cast<gate_kinds>(kind_);
	}

	std::uint32_t target() const
	{
		return target_;
	}

	std::uint32_t control() const
	{
		return control_;
	}

	std::uint32_t get_input_id(std::uint32_t qubit_id) const
	{
		if (target() == qubit_id) {
			return 0;
		}
		return 1;
	}

	bool is_control(std::uint32_t qubit_id) const 
	{
		if (qubit_id == 0) {
			return false;
		}
		return true;
	}

	bool is(gate_kinds kind) const
	{
		return kind_ == static_cast<std::uint8_t>(kind);
	}

	bool operator==(gate const& other) const
	{
		return data_ == other.data_;
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		fn(target_, 0);
	}

	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		if (this->is(gate_kinds::cnot)) {
			fn(control_, 1);
		}
	}

private:
	union {
		std::uint32_t data_;
		struct {
			std::uint32_t kind_ : 4;
			std::uint32_t control_ : 14;
			std::uint32_t target_ : 14;
		};
	};
};

public:
	void add_qubit(std::string const& qubit = {})
	{
		std::cout << "Add qubit: " << qubit << '\n';
	}

	void mark_as_input(std::string const& qubit)
	{
		std::cout << "Mark as input: " << qubit << '\n';
	}

	void mark_as_output(std::string const& qubit)
	{
		std::cout << "Mark as output: " << qubit << '\n';
	}

	void add_gate(gate_kinds kind, std::string const& target)
	{
		std::cout << "Add " << gate_name(kind)
		          << " gate to qubit: " << target << '\n';
	}

	void add_controlled_gate(gate_kinds kind, std::string const& control,
	                         std::string const& target)
	{
		std::cout << "Add " << gate_name(kind)
		          << " gate to qubits: " << control << ", " << target
		          << '\n';
	}
};

} // namespace tweedledum
