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

class expr_pi final : public ast_node {
public:
	static expr_pi* create(ast_context* ctx, uint32_t location)
	{
		return new (*ctx) expr_pi(location);
	}

	double evaluate() const
	{
		return M_PI;
	}

	double value() const
	{
		return M_PI;
	}

private:
	expr_pi(uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_pi;
	}
};

} // namespace quil
} // namespace tweedledee
