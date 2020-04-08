/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

namespace tweedledum::node {

// NOTE: When casted to an `uint32_t` this must return an index to the node
class id {
public:
	constexpr id()
	    : uid_(std::numeric_limits<uint32_t>::max())
	{}

	constexpr explicit id(uint32_t const uid)
	    : uid_(uid)
	{}

	operator uint32_t() const
	{
		return uid_;
	}

private:
	uint32_t uid_;
};

constexpr id invalid_id = id(std::numeric_limits<uint32_t>::max());

// NOTE:  This is used to wrap `operations` in the `netlist` represention of a quantum circuit.
template<typename Operation>
struct wrapper {
	Operation op;
	mutable uint32_t data;

	wrapper(Operation const& op, uint32_t const data_value)
	    : op(op)
	    , data(data_value)
	{}

	bool operator==(wrapper const& other) const
	{
		return op == other.op;
	}
};

template<typename Operation>
struct regular {
	Operation op;
	mutable uint32_t data;
	std::array<id, Operation::max_num_wires> children;

	regular(Operation const& op, uint32_t const data_value)
	    : op(op)
	    , data(data_value)
	{}

	bool operator==(regular const& other) const
	{
		return op == other.op;
	}
};

template<typename Operation>
struct irregular {
	Operation op;
	mutable uint32_t data;
	std::vector<id> children;

	irregular(Operation const& op, uint32_t const data_value)
	    : op(op)
	    , data(data_value)
	    , children(op.num_wires())
	{}

	bool operator==(irregular const& other) const
	{
		return op == other.op;
	}
};

} // namespace tweedledum::node
