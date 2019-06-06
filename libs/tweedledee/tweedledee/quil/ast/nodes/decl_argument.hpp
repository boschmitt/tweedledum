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

/*! \brief Argument declaration (decl) AST node

  Circuit definitions requires a list of formal arguments which can be
  substituted with either classical addresses or qubits.

*/
class decl_argument final : public ast_node {
public:
	static std::unique_ptr<decl_argument> build(std::uint32_t location,
	                                            std::string_view identifier)
	{
		auto result = std::unique_ptr<decl_argument>(
		    new decl_argument(location, std::move(identifier)));
		return result;
	}

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_argument(std::uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(std::move(identifier))
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_argument;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::green << "decl_argument "
		    << style::reset << fgB::cyan << identifier_ << fg::reset;
	}

private:
	std::string identifier_;
};

} // namespace quil
} // namespace tweedledee