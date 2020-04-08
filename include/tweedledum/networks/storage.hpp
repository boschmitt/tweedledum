/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "node.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {

template<typename Node>
struct storage {
	storage(std::string_view name_ = {})
	    : gate_set(0)
	    , name(name_)
	{}

	uint32_t default_value;
	uint64_t gate_set;
	std::string name;
	std::vector<node::id> inputs;
	std::vector<node::id> outputs;
	std::vector<Node> nodes;
};

} // namespace tweedledum
