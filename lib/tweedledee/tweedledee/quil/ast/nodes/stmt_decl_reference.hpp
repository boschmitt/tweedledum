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

namespace tweedledee {
namespace quil {

/*! \brief Declaration reference statement (stmt) AST node

  Links a invocation to a declaration

*/
class stmt_decl_reference final : public ast_node {
public:
	static std::unique_ptr<stmt_decl_reference>
	build(std::uint32_t location, const ast_node* decl)
	{
		auto result = std::unique_ptr<stmt_decl_reference>(
		    new stmt_decl_reference(location, decl));
		return result;
	}

private:
	stmt_decl_reference(unsigned location, const ast_node* decl)
	    : ast_node(location)
	    , declaration_(decl)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::stmt_decl_reference;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::magenta << "stmt_decl_reference "
		    << style::reset << fg::reset;
		declaration_->print(out);
	}

private:
	const ast_node* declaration_;
};

} // namespace quil
} // namespace tweedledee
