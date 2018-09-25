/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "detail/foreach.hpp"
#include "gates/gate_kinds.hpp"

#include <fmt/format.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <stack>
#include <vector>

namespace tweedledum {

template<typename G>
class netlist {
public:
	using gate_type = G;

	struct node_type {
		gate_type gate;
	};

public:
	using qubit = uint32_t;

	explicit netlist(uint32_t capacity = 32u)
	{
		assert(capacity <= 32u);
		for (int32_t i = capacity - 1; i >= 0; --i) {
			free_qubits_.push(i);
		}
	}

	auto num_qubits() const
	{
		return num_qubits_;
	}

	auto num_gates() const
	{
		return nodes_.size();
	}

	qubit allocate_qubit()
	{
		assert(!free_qubits_.empty());
		++current_qubits_;
		num_qubits_ = std::max(num_qubits_, current_qubits_);
		auto top = free_qubits_.top();
		free_qubits_.pop();
		return top;
	}

	void free_qubit(qubit q)
	{
		--current_qubits_;
		free_qubits_.push(q);
	}

	template<typename Fn>
	void foreach_node(Fn&& fn) const
	{
		for (auto& n : nodes_) {
			fn(n);
		}
	}

	node_type& add_gate(gate_type const& g)
	{
		auto& n = nodes_.emplace_back();
		n.gate = g;
		return n;
	}

	void add_gate(gate_kinds_t kind, uint32_t target)
	{
		auto& n = nodes_.emplace_back();
		auto& gate = n.gate;
		if (kind == gate_kinds_t::pauli_x || kind == gate_kinds_t::cx || kind == gate_kinds_t::mcx)
			gate.kind(gate_kinds_t::mcx);
		else if (kind == gate_kinds_t::pauli_z || kind == gate_kinds_t::cz || kind == gate_kinds_t::mcz)
			gate.kind(gate_kinds_t::mcz);
		else
			gate.kind(kind);
		gate.targets |= 1 << target;
	}

	void add_controlled_gate(gate_kinds_t kind, uint32_t control,
	                         uint32_t target)
	{
		assert(kind == gate_kinds_t::cx || kind == gate_kinds_t::cz);
		assert(control != target);
		auto& n = nodes_.emplace_back();
		auto& gate = n.gate;
		if (kind == gate_kinds_t::pauli_x || kind == gate_kinds_t::cx || kind == gate_kinds_t::mcx)
			gate.kind(gate_kinds_t::mcx);
		else if (kind == gate_kinds_t::pauli_z || kind == gate_kinds_t::cz || kind == gate_kinds_t::mcz)
			gate.kind(gate_kinds_t::mcz);
		else
			gate.kind(kind);
		gate.controls |= 1 << control;
		gate.targets |= 1 << target;
	}

	// first item in qubits is target
	void add_multiple_controlled_gate(gate_kinds_t kind,
	                                  std::vector<uint32_t> const& qubits)
	{
		assert(!qubits.empty());
		auto& n = nodes_.emplace_back();
		auto& gate = n.gate;
		gate.kind(kind);
		std::for_each(qubits.begin() + 1, qubits.end(),
		              [&](auto q) { gate.controls |= 1 << q; });
		gate.targets |= 1 << qubits.front();
	}

	node_type& add_multiple_controlled_target_gate(gate_kinds_t kind,
	                                               uint32_t controls,
	                                               uint32_t targets)
	{
		auto& n = nodes_.emplace_back();
		auto& gate = n.gate;
		gate.controls = controls;
		gate.targets = targets;
		gate.kind(kind);
		return n;
	}

	node_type& add_multiple_controlled_target_gate(
	    gate_kinds_t kind, std::vector<qubit> const& controls,
	    std::vector<qubit> const& targets)
	{
		auto& n = nodes_.emplace_back();
		auto& gate = n.gate;
		gate.kind(kind);
		std::for_each(controls.begin(), controls.end(),
		              [&](auto q) { gate.controls |= 1 << q; });
		std::for_each(targets.begin(), targets.end(),
		              [&](auto q) { gate.targets |= 1 << q; });
		return n;
	}

	template<typename Fn>
	void foreach_qubit(Fn&& fn) const
	{
		for (auto index = 0u; index < current_qubits_; ++index) {
			std::string label = fmt::format("q{}", index);
			fn(index, label);
		}
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn) const
	{
		auto index = 0u;
		auto begin = nodes_.begin();
		auto end = nodes_.end();
		detail::foreach_element(begin, end, fn, index);
	}

	auto mark(node_type const& n) const
	{
		(void) n;
		return 0;
	}

private:
	uint32_t current_qubits_ = 0u;
	uint32_t num_qubits_ = 0u;
	std::stack<uint32_t> free_qubits_;
	std::vector<node_type> nodes_;
};

}; // namespace tweedledum
