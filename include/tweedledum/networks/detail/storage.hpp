/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../io_id.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {
namespace detail {

/*! \brief Use to 'point' to a node
 *
 * Users may define their own `link` structure, which may hold more information than just an index
 *
 * NOTE:  When casted to `uint32_t`, this type should return a valid index within the vector
 * holding the vertecies of the graph
 */
struct link {
	static constexpr auto max = std::numeric_limits<uint32_t>::max();

	constexpr link()
	    : index(max)
	{}

	constexpr explicit link(uint32_t index)
	    : index(index)
	{}

	operator uint32_t() const
	{
		return index;
	}

private:
	uint32_t index;
};

} // namespace detail

// NOTE:  This is used to wrap `gates` in the `netlist` represention of a quantum circuit.
template<typename GateType, int DataSize = 0, typename LinkType = detail::link>
struct wrapper_vertex {
	using link_type = LinkType;

	GateType gate;
	mutable std::array<uint32_t, DataSize> data;

	wrapper_vertex(GateType const& gate_)
	    : gate(gate_)
	{}

	bool operator==(wrapper_vertex const& other) const
	{
		return gate == other.gate;
	}
};

template<typename GateType, int DataSize = 0, typename LinkType = detail::link>
struct node {
	using link_type = LinkType;

	GateType gate;
	std::array<link_type, GateType::max_num_io> children;
	mutable std::array<uint32_t, DataSize> data;

	node(GateType const& gate_)
	    : gate(gate_)
	{}

	bool operator==(node const& other) const
	{
		return gate == other.gate;
	}
};

template<typename VertexType>
struct storage {
	storage(std::string_view name_ = {})
	    : name(name_)
	    , num_qubits(0)
	    , gate_set(0)
	{
		nodes.reserve(1024u);
	}

	std::string name;
	uint32_t num_qubits;
	uint32_t gate_set;
	uint32_t default_value;
	std::vector<uint32_t> inputs;
	std::vector<VertexType> nodes;
	std::vector<VertexType> outputs;
	std::vector<io_id> wiring_map;
	std::vector<uint8_t> io_marks;
};

class labels_map {
public:
	void map(io_id id, std::string const& label)
	{
		label_to_id_.emplace(label, id);
		id_to_label_.emplace_back(label, id);
	}

	void remap(io_id id, std::string const& label)
	{
		label_to_id_.emplace(label, id);
		id_to_label_.at(id) = std::make_pair(label, id);
	}

	io_id to_id(std::string const& label) const
	{
		return label_to_id_.at(label);
	}

	std::string to_label(io_id id) const
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
	std::unordered_map<std::string, io_id> label_to_id_;
	std::vector<std::pair<std::string, io_id>> id_to_label_;
};

} // namespace tweedledum
