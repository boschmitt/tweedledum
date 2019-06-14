/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_context.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>

namespace tweedledee {
namespace qasm {

// This represents a variable parameter in a gate declaration (`decl_gate`)
class decl_param final : public ast_node {
public:
	static decl_param* build(ast_context* ctx, uint32_t location, std::string_view identifier)
	{
		auto result = new (*ctx) decl_param(location, identifier);
		return result;
	}

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_param(uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(identifier)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_param;
	}

private:
	std::string identifier_;
};

} // namespace qasm
} // namespace tweedledee
