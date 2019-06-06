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

/*! \brief Qubit AST node

  A qubit is referred to by its integer index.
  For example, Q5 is referred to by 5.

*/
class qubit final : public ast_node {
public:
	static std::unique_ptr<qubit> build(uint32_t location, std::string_view label)
	{
		auto result = std::unique_ptr<qubit>(new qubit(location, label));
		return result;
	}

	std::string label;

private:
	qubit(uint32_t location, std::string_view label)
	    : ast_node(location)
	    , label(label)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::qubit;
	}

	void do_print(std::ostream& out) const override
	{
		out << "qubit " << label;
	}
};

} // namespace quil
} // namespace tweedledee
