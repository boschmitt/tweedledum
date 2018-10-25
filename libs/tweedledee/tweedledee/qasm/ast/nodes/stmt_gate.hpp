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

class stmt_gate
    : public ast_node
    , public ast_node_container<stmt_gate, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location)
		    : statement_(new stmt_gate(location))
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

private:
	stmt_gate(std::uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::stmt_gate;
	}

	void do_print(std::ostream& out) const override
	{
		out << "stmt_gate\n";
	}
};

} // namespace qasm
} // namespace tweedledee
