/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
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

template<typename G, int NumQubits = 2, int PointerFieldSize = 1>
struct regular_node {
	using gate_type = G;
	using pointer_type = node_pointer<PointerFieldSize>;

	gate_type gate;
	std::array<std::vector<pointer_type>, NumQubits> qubit;

	bool operator==(regular_node<G, NumQubits> const& other) const
	{
		return gate == other.gate;
	}
};

template<typename G, int NumQubits = 2, int PointerFieldSize = 1>
struct uniform_node {
	using gate_type = G;
	using pointer_type = node_pointer<PointerFieldSize>;

	gate_type gate;
	std::array<std::array<pointer_type, 2>, NumQubits> qubit;

	bool operator==(uniform_node<G, NumQubits> const& other) const
	{
		return gate == other.gate;
	}
};

} // namespace tweedledum
