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

//
class expr_decl_ref final : public ast_node {

public:
	static std::unique_ptr<expr_decl_ref> build(std::uint32_t location, ast_node* decl)
	{
		auto result = std::unique_ptr<expr_decl_ref>(new expr_decl_ref(location, decl));
		return result;
	}

private:
	expr_decl_ref(unsigned location, ast_node* decl)
	    : ast_node(location)
	    , decl_(decl)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_decl_ref;
	}

	void do_print(std::ostream& out) const override
	{
		out << "expr_decl_ref\n";
	}

private:
	ast_node* decl_;
};

} // namespace qasm
} // namespace tweedledee
