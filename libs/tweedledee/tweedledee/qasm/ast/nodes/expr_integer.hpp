/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>

namespace tweedledee {
namespace qasm {

class expr_integer final : public ast_node {

public:
	static std::unique_ptr<expr_integer> build(uint32_t location, int32_t value)
	{
		auto result = std::unique_ptr<expr_integer>(new expr_integer(location, value));
		return result;
	}

	int32_t evaluate() const
	{
		return value_;
	}

private:
	expr_integer(uint32_t location, int32_t value)
	    : ast_node(location)
	    , value_(value)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_integer;
	}

private:
	int32_t value_;
};

} // namespace qasm
} // namespace tweedledee
