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

//
class expr_reg_idx_ref
    : public ast_node
    , public ast_node_container<expr_reg_idx_ref, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location)
		    : statement_(new expr_reg_idx_ref(location))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			statement_->add_child(std::move(child));
		}

		expr_reg_idx_ref& get()
		{
			return *statement_;
		}

		std::unique_ptr<expr_reg_idx_ref> finish()
		{
			return std::move(statement_);
		}

	private:
		std::unique_ptr<expr_reg_idx_ref> statement_;
	};

private:
	expr_reg_idx_ref(std::uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_reg_idx_ref;
	}

	void do_print(std::ostream& out) const override
	{
		out << "expr_reg_idx_ref\n";
	}
};

} // namespace qasm
} // namespace tweedledee
