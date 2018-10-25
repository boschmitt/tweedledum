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
class expr_binary_op
    : public ast_node
    , public ast_node_container<expr_binary_op, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location, char op)
		    : expression_(new expr_binary_op(location, op))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			expression_->add_child(std::move(child));
		}

		expr_binary_op& get()
		{
			return *expression_;
		}

		std::unique_ptr<expr_binary_op> finish()
		{
			return std::move(expression_);
		}

	private:
		std::unique_ptr<expr_binary_op> expression_;
	};

private:
	expr_binary_op(std::uint32_t location, char op)
	    : ast_node(location)
	    , operator_(op)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_binary_op;
	}

	void do_print(std::ostream& out) const override
	{
		out << "expr_binary_op '" << operator_ << "'\n";
	}

private:
	char operator_;
};

} // namespace qasm
} // namespace tweedledee
