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

namespace tweedledee {
namespace quil {

/*! \brief Integer number expression AST node
 */
class expr_integer final : public ast_node {
public:
	static std::unique_ptr<expr_integer> build(uint32_t location, int32_t value, bool imaginary)
	{
		auto result = std::unique_ptr<expr_integer>(new expr_integer(location, value, imaginary));
		return result;
	}

	auto evaluate() const
	{
		return value_;
	}

private:
	expr_integer(uint32_t location, int32_t value, bool imaginary)
	    : ast_node(location)
	    , value_(value)
	    , imaginary_(imaginary)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_integer;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::magenta << "expr_integer " << style::reset << fg::cyan
		    << value_ << fg::reset;
	}

private:
	int32_t value_;
	bool imaginary_;
};

} // namespace quil
} // namespace tweedledee
