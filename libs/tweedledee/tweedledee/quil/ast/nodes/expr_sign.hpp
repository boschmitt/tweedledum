/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
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

/*! \brief Sign of an expression AST node
 */
class expr_sign
    : public ast_node
    , public ast_node_container<expr_sign, ast_node> {
public:
	class builder {
	public:
		explicit builder(uint32_t location, char sign)
		    : expression_(new expr_sign(location, sign))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			expression_->add_child(std::move(child));
		}

		expr_sign& get()
		{
			return *expression_;
		}

		std::unique_ptr<expr_sign> finish()
		{
			return std::move(expression_);
		}

	private:
		std::unique_ptr<expr_sign> expression_;
	};

private:
	expr_sign(uint32_t location, char sign)
	    : ast_node(location)
	    , sign_(sign)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_sign;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::magenta << "expr_sign " << style::reset << fg::reset
		    << '\'' << sign_ << '\'';
	}

private:
	char sign_;
};

} // namespace quil
} // namespace tweedledee
