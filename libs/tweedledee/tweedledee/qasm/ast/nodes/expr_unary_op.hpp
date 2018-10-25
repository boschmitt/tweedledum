/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <ostream>
#include <string>

namespace tweedledee {
namespace qasm {

// TODO: maybe move this out of here.
enum class unary_ops : unsigned short {
	sin,
	cos,
	tan,
	exp,
	ln,
	sqrt,
	minus,
	plus,
	unknown,
};

static const std::string unray_op_names[] = {
    "sin", "cos", "tan", "exp", "ln", "sqrt", "minus", "plus", "unknown",
};

static std::string_view unray_op_name(unary_ops op)
{
	auto op_idx = static_cast<unsigned short>(op);
	return unray_op_names[op_idx];
}

//
class expr_unary_op
    : public ast_node
    , public ast_node_container<expr_unary_op, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location, unary_ops op)
		    : expression_(new expr_unary_op(location, op))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			expression_->add_child(std::move(child));
		}

		expr_unary_op& get()
		{
			return *expression_;
		}

		std::unique_ptr<expr_unary_op> finish()
		{
			return std::move(expression_);
		}

	private:
		std::unique_ptr<expr_unary_op> expression_;
	};

private:
	expr_unary_op(std::uint32_t location, unary_ops op)
	    : ast_node(location)
	    , operator_(op)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_unary_op;
	}

	void do_print(std::ostream& out) const override
	{
		out << "expr_unary_op '" << unray_op_name(operator_) << "'\n";
	}

private:
	unary_ops operator_;
};

} // namespace qasm
} // namespace tweedledee
