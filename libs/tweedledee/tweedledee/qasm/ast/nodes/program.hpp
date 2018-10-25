/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>

namespace tweedledee {
namespace qasm {

// Root node class of the AST
class program final
    : public ast_node
    , public ast_node_container<program, ast_node> {
public:
	class builder {
	public:
		explicit builder()
		    : program_(new program())
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			program_->add_child(std::move(child));
		}

		program& get()
		{
			return *program_;
		}

		std::unique_ptr<program> finish()
		{
			return std::move(program_);
		}

	private:
		std::unique_ptr<program> program_;
	};

private:
	program()
	    : ast_node(0)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::program;
	}

	void do_print(std::ostream& out) const override
	{
		(void) out;
	}
};

} // namespace qasm
} // namespace tweedledee
