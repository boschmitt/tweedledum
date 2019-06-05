/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <rang/rang.hpp>
#include <string>

namespace tweedledee {
namespace quil {

/*! \brief Parameter declaration (decl) AST node

  Formal parameters are names prepended with a ‘%’ symbol, which can be defined
  in a gate and circuit declarations.

*/
class decl_parameter final : public ast_node {
public:
	static std::unique_ptr<decl_parameter>
	build(std::uint32_t location, std::string_view identifier)
	{
		auto result = std::unique_ptr<decl_parameter>(
		    new decl_parameter(location, std::move(identifier)));
		return result;
	}

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_parameter(std::uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(std::move(identifier))
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_parameter;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::green << "decl_parameter " << style::reset << fgB::cyan
		    << identifier_ << fg::reset;
	}

private:
	std::string identifier_;
};

} // namespace quil
} // namespace tweedledee
