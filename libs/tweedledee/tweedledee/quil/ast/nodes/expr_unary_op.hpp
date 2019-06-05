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

// TODO: maybe move this out of here.
enum class unary_ops : unsigned short {
	sin,
	cos,
	cis,
	sqrt,
	exp,
	minus,
	plus,
	unknown,
};

static const std::string unray_op_names[] = {
    "sin", "cos", "cis", "sqrt", "exp", "minus", "plus", "unknown",
};

/*! \brief Unary operator expression AST node
 */
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
		using namespace rang;
		out << style::bold << fgB::magenta << "expr_unary_op " << style::reset << fg::reset
		    << '\'' << unray_op_names[static_cast<uint32_t>(operator_)] << '\'';
	}

private:
	unary_ops operator_;
};

} // namespace quil
} // namespace tweedledee
