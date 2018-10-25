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

enum class RegType : unsigned short {
	quantum,
	classical,
};

// This represents a register (quantum or classical) declaration
class decl_register final : public ast_node {
public:
	static std::unique_ptr<decl_register> build(std::uint32_t location, RegType type,
	                                            std::string_view identifier, std::uint32_t size)
	{
		auto result = std::unique_ptr<decl_register>(
		    new decl_register(location, type, std::move(identifier), size));
		return result;
	}

	RegType type() const
	{
		return type_;
	}

	std::string_view identifier() const
	{
		return identifier_;
	}

	std::uint32_t size() const
	{
		return size_;
	}

private:
	decl_register(std::uint32_t location, RegType type, std::string_view identifier,
	              std::uint32_t size)
	    : ast_node(location)
	    , type_(type)
	    , identifier_(identifier)
	    , size_(size)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_register;
	}

	void do_print(std::ostream& out) const override
	{
		out << "decl_register " << identifier_ << "\n";
	}

private:
	RegType type_;
	std::string identifier_;
	std::uint32_t size_;
};

} // namespace qasm
} // namespace tweedledee
