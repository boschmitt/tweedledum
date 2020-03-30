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

// NOTE: When casted to an `uint32_t` this must return an index to the node
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

namespace node {
constexpr node_id invalid = node_id(std::numeric_limits<uint32_t>::max());
} // namespace node

template<typename Node>
struct storage {
	storage(std::string_view name_ = {})
	    : gate_set(0)
	    , name(name_)
	{ }

	uint32_t default_value;
	uint64_t gate_set;
	std::string name;
	std::vector<node_id> inputs;
	std::vector<node_id> outputs;
	std::vector<Node> nodes;
};

} // namespace tweedledum
