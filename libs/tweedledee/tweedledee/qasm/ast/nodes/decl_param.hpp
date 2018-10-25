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
class decl_param final : public ast_node {
public:
	static std::unique_ptr<decl_param> build(std::uint32_t location, std::string_view identifier)
	{
		auto result = std::unique_ptr<decl_param>(
		    new decl_param(location, std::move(identifier)));
		return result;
	}

	std::string identifier() const
	{
		return identifier_;
	}

private:
	decl_param(std::uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(std::move(identifier))
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_param;
	}

	void do_print(std::ostream& out) const override
	{
		out << "decl_param " << identifier_ << "\n";
	}

private:
	std::string identifier_;
};

} // namespace qasm
} // namespace tweedledee
