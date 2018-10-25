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

// This represents a register (quantum or classical) declaration
class expr_integer final : public ast_node {

public:
	static std::unique_ptr<expr_integer> build(std::uint32_t location, std::int32_t value)
	{
		auto result = std::unique_ptr<expr_integer>(new expr_integer(location, value));
		return result;
	}

private:
	expr_integer(std::uint32_t location, std::int32_t value)
	    : ast_node(location)
	    , value_(value)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_integer;
	}

	void do_print(std::ostream& out) const override
	{
		out << "expr_integer " << value_ << "\n";
	}

private:
	std::int32_t value_;
};

} // namespace qasm
} // namespace tweedledee
