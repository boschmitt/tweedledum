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
class expr_real final : public ast_node {
public:
	static std::unique_ptr<expr_real> build(std::uint32_t location, double value)
	{
		auto result = std::unique_ptr<expr_real>(new expr_real(location, value));
		return result;
	}

private:
	expr_real(std::uint32_t location, double value)
	    : ast_node(location)
	    , value_(value)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_real;
	}

	void do_print(std::ostream& out) const override
	{
		out << "expr_real " << value_ << "\n";
	}

private:
	double value_;
};

} // namespace qasm
} // namespace tweedledee
