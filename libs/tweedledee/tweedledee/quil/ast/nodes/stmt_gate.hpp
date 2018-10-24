/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gate_kinds.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <rang/rang.hpp>

namespace tweedledee {
namespace quil {

/*! \brief Gate statement (stmt) AST node
 */
class stmt_gate
    : public ast_node
    , public ast_node_container<stmt_gate, ast_node> {
public:
	class builder {
	public:
		explicit builder(uint32_t location, std::string_view identifier)
		    : statement_(new stmt_gate(location, identifier))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			statement_->add_child(std::move(child));
		}

		stmt_gate& get()
		{
			return *statement_;
		}

		std::unique_ptr<stmt_gate> finish()
		{
			return std::move(statement_);
		}

	private:
		std::unique_ptr<stmt_gate> statement_;
	};

	std::string identifier;

private:
	stmt_gate(uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier(identifier)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::stmt_gate;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::magenta << "stmt_gate " << style::reset << fgB::cyan
		    << identifier << fg::reset;
	}
};

} // namespace quil
} // namespace tweedledee
