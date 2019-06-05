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

/*! \brief Gate declaration (decl) AST node

  In Quil, every gate is defined separately from its invocation.
  There are two gate-related concepts in Quil: static and parametric gates.
  A static gate is an operator in U(2Nq ), and a parametric gate is a function
  Cn -> U(2Nq).

*/
class decl_gate
    : public ast_node
    , public ast_node_container<decl_gate, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location,
		                 std::string_view identifier)
		    : declaration_(new decl_gate(location, identifier))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			declaration_->add_child(std::move(child));
		}

		decl_gate& get()
		{
			return *declaration_;
		}

		std::unique_ptr<decl_gate> finish()
		{
			return std::move(declaration_);
		}

	private:
		std::unique_ptr<decl_gate> declaration_;
	};

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_gate(std::uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(identifier)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_gate;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::green << "decl_gate " << style::reset
		    << fgB::cyan << identifier_ << fg::reset;
	}

private:
	std::string identifier_;
};

} // namespace quil
} // namespace tweedledee
