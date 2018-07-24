/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <limits>
#include <cstdint>
#include <vector>

namespace tweedledum {

template<int PointerFieldSize = 0>
struct node_pointer {
private:
	static constexpr auto length = sizeof(std::uint32_t) * 8;

public:
	static constexpr auto max = std::numeric_limits<std::uint32_t>::max();

	node_pointer()
	    : data(max)
	{}

	node_pointer(std::uint32_t data)
	    : data(data)
	{}

	node_pointer(std::uint32_t index, std::uint32_t weight)
	    : weight(weight)
	    , index(index)
	{}

	union {
		std::uint32_t data;
		struct {
			std::uint32_t weight : PointerFieldSize;
			std::uint32_t index : length - PointerFieldSize;
		};
	};

	bool operator==(node_pointer<PointerFieldSize> const& other) const
	{
		return data == other.data;
	}
};

union cauint32_t {
	std::uint32_t w{0};
	struct {
		std::uint32_t b0 : 8;
		std::uint32_t b1 : 8;
		std::uint32_t b2 : 8;
		std::uint32_t b3 : 8;
	};
};

template<typename GateType, int PointerFieldSize = 1, int DataSize = 0>
struct regular_node {
	using pointer_type = node_pointer<PointerFieldSize>;

	GateType gate;
	std::array<std::vector<pointer_type>, GateType::max_num_qubits> qubit;
	std::array<cauint32_t, DataSize> data;

	bool operator==(regular_node<GateType, PointerFieldSize, DataSize> const& other) const
	{
		return gate == other.gate;
	}
};

template<typename GateType, int PointerFieldSize = 1, int DataSize = 0>
struct uniform_node {
	using pointer_type = node_pointer<PointerFieldSize>;

	GateType gate;
	std::array<std::array<pointer_type, 2>, GateType::max_num_qubits> qubit;
	std::array<cauint32_t, DataSize> data;

	bool operator==(uniform_node<GateType, PointerFieldSize, DataSize> const& other) const
	{
		return gate == other.gate;
	}
};

template<typename NodeType>
struct storage {
	storage()
	{
		nodes.reserve(1024u);
	}

	storage(std::uint32_t size)
	{
		nodes.reserve(size);
	}

	std::vector<typename NodeType::pointer_type> inputs;
	std::vector<NodeType> nodes;
	std::vector<NodeType> outputs;
};

} // namespace tweedledum
