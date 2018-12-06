/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../qubit.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {
namespace detail {

/*! \brief 
 */
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

	bool operator==(node_pointer const& other) const
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

} // namespace detail

union cauint32_t {
	uint32_t w{0};
	struct {
		uint32_t b0 : 8;
		uint32_t b1 : 8;
		uint32_t b2 : 8;
		uint32_t b3 : 8;
	};
};

/*! \brief 
 */
template<typename GateType, int DataSize = 0>
struct wrapper_node {
	using pointer_type = detail::node_pointer<0>;

	GateType gate;
	mutable std::array<cauint32_t, DataSize> data;

	wrapper_node(GateType const& g)
	    : gate(g)
	{}

	bool operator==(wrapper_node const& other) const
	{
		return gate == other.gate;
	}
};

/*! \brief 
 */
template<typename GateType, int PointerFieldSize = 1, int DataSize = 0>
struct regular_node {
	using pointer_type = detail::node_pointer<PointerFieldSize>;

	GateType gate;
	std::array<std::vector<pointer_type>, GateType::max_num_qubits> qubit;
	mutable std::array<cauint32_t, DataSize> data;

	regular_node(GateType const& g)
	    : gate(g)
	{}

	bool operator==(regular_node const& other) const
	{
		return gate == other.gate;
	}
};

/*! \brief 
 */
template<typename GateType, int PointerFieldSize = 1, int DataSize = 0>
struct uniform_node {
	using pointer_type = detail::node_pointer<PointerFieldSize>;

	GateType gate;
	std::array<std::array<pointer_type, 2>, GateType::max_num_qubits> qubit;
	mutable std::array<cauint32_t, DataSize> data;

	uniform_node(GateType const& g)
	    : gate(g)
	{}

	bool operator==(uniform_node const& other) const
	{
		return gate == other.gate;
	}
};

/*! \brief 
 */
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

	std::vector<uint32_t> inputs;
	std::vector<NodeType> nodes;
	std::vector<NodeType> outputs;
	std::vector<uint32_t> rewiring_map;
};

/*! \brief 
 */
class qlabels_map {
public:
	auto map(qubit_id qid, std::string const& qlabel)
	{
		qlabel_to_qid_.emplace(qlabel, qid);
		qid_to_qlabel_.emplace_back(qlabel);
	}

	auto to_qid(std::string const& qlabel) const
	{
		return qlabel_to_qid_.at(qlabel);
	}

	auto to_qlabel(qubit_id qid) const
	{
		return qid_to_qlabel_.at(qid);
	}

	auto cbegin() const
	{
		return qid_to_qlabel_.cbegin();
	}

	auto cend() const
	{
		return qid_to_qlabel_.cend();
	}

	auto begin()
	{
		return qid_to_qlabel_.begin();
	}

	auto end()
	{
		return qid_to_qlabel_.end();
	}

private:
	std::unordered_map<std::string, qubit_id> qlabel_to_qid_;
	std::vector<std::string> qid_to_qlabel_;
};

} // namespace tweedledum
