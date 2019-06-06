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

/*! \brief Circuit declaration (decl) AST node

  Sometimes it is convenient to name and parameterize a particular sequence of
  Quil instructions for use as a subroutine to other quantum programs. This can
  be done by declaring circuits (DEFCIRCUIT).

*/
class decl_circuit
    : public ast_node
    , public ast_node_container<decl_circuit, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location,
		                 std::string_view identifier)
		    : statement_(new decl_circuit(location, identifier))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			statement_->add_child(std::move(child));
		}

		decl_circuit& get()
		{
			return *statement_;
		}

		std::unique_ptr<decl_circuit> finish()
		{
			return std::move(statement_);
		}

	private:
		std::unique_ptr<decl_circuit> statement_;
	};

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_circuit(std::uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(identifier)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_circuit;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::green << "decl_circuit "
		    << style::reset << fgB::cyan << identifier_ << fg::reset;
	}

private:
	std::string identifier_;
};

} // namespace quil
} // namespace tweedledee

