/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_context.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

namespace tweedledee {
namespace quil {

class expr_integer final : public ast_node {

public:
	static expr_integer* create(ast_context* ctx, uint32_t location, int32_t value)
	{
		return new (*ctx) expr_integer(location, value);
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

} // namespace quil
} // namespace tweedledee
