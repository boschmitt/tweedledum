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
#include <string>

namespace tweedledee {
namespace quil {

enum class memory_types : unsigned short {
	bit,
	real,
	octet,
	integer,
	none,
};

/*! \brief Argument declaration (decl) AST node

  Circuit definitions requires a list of formal arguments which can be
  substituted with either classical addresses or qubits.

*/
class decl_memory final : public ast_node {
public:
	static std::unique_ptr<decl_memory>
	build(uint32_t location, std::string_view identifier, memory_types type_, uint32_t size_)
	{
		auto result = std::unique_ptr<decl_memory>(
		    new decl_memory(location, std::move(identifier), type_, size_));
		return result;
	}

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_memory(std::uint32_t location, std::string_view identifier, memory_types mem_type,
	            uint32_t size)
	    : ast_node(location)
	    , identifier_(std::move(identifier))
	    , type_(mem_type)
	    , size_(size)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_memory;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << style::bold << fgB::green << "decl_memory " << style::reset << fgB::cyan
		    << identifier_ << fg::reset;
	}

private:
	std::string identifier_;
	memory_types type_;
	uint32_t size_;
};

} // namespace quil
} // namespace tweedledee