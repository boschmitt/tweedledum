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

namespace tweedledee {
namespace quil {

/*! \brief Real number expression AST node
 */
class expr_real final : public ast_node {
public:
	static std::unique_ptr<expr_real> build(uint32_t location, float value, bool imaginary)
	{
		auto result = std::unique_ptr<expr_real>(new expr_real(location, value, imaginary));
		return result;
	}

	auto evaluate() const
	{
		return value_;
	}

private:
	expr_real(uint32_t location, float value, bool imaginary)
	    : ast_node(location)
	    , value_(value)
	    , imaginary_(imaginary)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_real;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::magenta << "expr_real " << style::reset << fg::cyan
		    << value_ << fg::reset;
	}

private:
	float value_;
	bool imaginary_;
};

} // namespace quil
} // namespace tweedledee
