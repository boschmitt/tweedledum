/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_kinds.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {

template<int PointerFieldSize = 0>
struct node_pointer {
private:
	static constexpr auto length = sizeof(uint32_t) * 8;

public:
	static constexpr auto max = std::numeric_limits<uint32_t>::max();

	node_pointer()
	    : data(max)
	{}

	node_pointer(uint32_t data)
	    : data(data)
	{}

	node_pointer(uint32_t index, uint32_t weight)
	    : weight(weight)
	    , index(index)
	{}

	union {
		uint32_t data;
		struct {
			uint32_t weight : PointerFieldSize;
			uint32_t index : length - PointerFieldSize;
		};
	};

	bool operator==(node_pointer<PointerFieldSize> const& other) const
	{
		return data == other.data;
	}
};

template<>
struct node_pointer<0> {
	static constexpr auto max = std::numeric_limits<uint32_t>::max();

	node_pointer()
	    : data(max)
	{}

	node_pointer(uint32_t data)
	    : data(data)
	{}

	union {
		uint32_t data;
		uint32_t index;
	};

	bool operator==(node_pointer const& other) const
	{
		return data == other.data;
	}
};

union cauint32_t {
	uint32_t w{0};
	struct {
		uint32_t b0 : 8;
		uint32_t b1 : 8;
		uint32_t b2 : 8;
		uint32_t b3 : 8;
	};
};

template<typename GateType, int DataSize = 0>
struct wrapper_node {
	using pointer_type = node_pointer<0>;

	GateType gate;
	mutable std::array<cauint32_t, DataSize> data;

	wrapper_node(GateType const& g)
	    : gate(g)
	{}

	wrapper_node(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0)
	    : gate(kind, target, rotation_angle)
	{}

	wrapper_node(gate_kinds_t kind, std::vector<uint32_t>& controls, std::vector<uint32_t>& targets,
	             float rotation_angle = 0.0)
	    : gate(kind, controls, targets, rotation_angle)
	{}

	bool operator==(wrapper_node const& other) const
	{
		return gate == other.gate;
	}
};

//
template<typename GateType, int PointerFieldSize = 1, int DataSize = 0>
struct regular_node {
	using pointer_type = node_pointer<PointerFieldSize>;

	GateType gate;
	std::array<std::vector<pointer_type>, GateType::max_num_qubits> qubit;
	mutable std::array<cauint32_t, DataSize> data;

	regular_node(GateType const& g)
	    : gate(g)
	{}

	regular_node(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0)
	    : gate(kind, target, rotation_angle)
	{}

	regular_node(gate_kinds_t kind, std::vector<uint32_t>& controls, std::vector<uint32_t>& targets,
	             float rotation_angle = 0.0)
	    : gate(kind, controls, targets, rotation_angle)
	{}

	bool operator==(regular_node const& other) const
	{
		return gate == other.gate;
	}
};

//
template<typename GateType, int PointerFieldSize = 1, int DataSize = 0>
struct uniform_node {
	using pointer_type = node_pointer<PointerFieldSize>;

	GateType gate;
	std::array<std::array<pointer_type, 2>, GateType::max_num_qubits> qubit;
	mutable std::array<cauint32_t, DataSize> data;

	uniform_node(GateType const& g)
	    : gate(g)
	{}

	uniform_node(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0)
	    : gate(kind, target, rotation_angle)
	{}

	uniform_node(gate_kinds_t kind, std::vector<uint32_t>& controls, std::vector<uint32_t>& targets,
	             float rotation_angle = 0.0)
	    : gate(kind, controls, targets, rotation_angle)
	{}

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

	storage(uint32_t size)
	{
		nodes.reserve(size);
	}

	std::vector<typename NodeType::pointer_type> inputs;
	std::vector<NodeType> nodes;
	std::vector<NodeType> outputs;
	// Qubit names
	std::unordered_map<std::string, uint32_t> label_to_id;
	std::vector<std::string> id_to_label;
};

} // namespace tweedledum
