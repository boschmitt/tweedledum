/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "wire_id.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {

class node_id {
public:
	constexpr node_id()
	    : id_(std::numeric_limits<uint32_t>::max())
	{}

	constexpr explicit node_id(uint32_t id)
	    : id_(id)
	{}

	operator uint32_t() const
	{
		return id_;
	}

private:
	uint32_t id_;
};

namespace node {
constexpr node_id invalid = node_id(std::numeric_limits<uint32_t>::max());
} // namespace node

// NOTE:  This is used to wrap `gates` in the `netlist` represention of a quantum circuit.
template<typename Operation>
struct node_wrapper {
	Operation op;
	mutable uint32_t data;

	node_wrapper(Operation const& op, uint32_t data_value)
	    : op(op)
	    , data(data_value)
	{}

	bool operator==(node_wrapper const& other) const
	{
		return op == other.op;
	}
};

template<typename Operation>
struct node_regular {
	Operation op;
	mutable uint32_t data;
	std::array<node_id, Operation::max_num_wires> children;

	node_regular(Operation const& op, uint32_t data_value)
	    : op(op)
	    , data(data_value)
	{}

	bool operator==(node_regular const& other) const
	{
		return op == other.op;
	}
};

template<typename Operation>
struct node_irregular {
	Operation op;
	mutable uint32_t data;
	std::vector<node_id> children;

	node_irregular(Operation const& op, uint32_t data_value)
	    : op(op)
	    , data(data_value)
	    , children(op.num_wires())
	{}

	bool operator==(node_irregular const& other) const
	{
		return op == other.op;
	}
};

template<typename Node>
struct storage {
	storage(std::string_view name_ = {})
	    : num_qubits(0)
	    , gate_set(0)
	    , name(name_)
	{ }

	uint32_t default_value;
	uint32_t num_qubits;
	uint64_t gate_set;
	std::string name;
	std::vector<node_id> inputs;
	std::vector<node_id> outputs;
	std::vector<Node> nodes;

	std::vector<wire_modes> wire_mode;
};

class labels_map {
public:
	void map(wire_id id, std::string const& label)
	{
		label_to_id_.emplace(label, id);
		id_to_label_.emplace_back(label, id);
	}

	void remap(wire_id id, std::string const& label)
	{
		label_to_id_.emplace(label, id);
		id_to_label_.at(id) = std::make_pair(label, id);
	}

	wire_id to_id(std::string const& label) const
	{
		return label_to_id_.at(label);
	}

	std::string to_label(wire_id id) const
	{
		return id_to_label_.at(id).first;
	}

	auto cbegin() const
	{
		return id_to_label_.cbegin();
	}

	auto cend() const
	{
		return id_to_label_.cend();
	}

	auto begin()
	{
		return id_to_label_.begin();
	}

	auto end()
	{
		return id_to_label_.end();
	}

private:
	std::unordered_map<std::string, wire_id> label_to_id_;
	std::vector<std::pair<std::string, wire_id>> id_to_label_;
};

} // namespace tweedledum
