/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>
#include <vector>

namespace tweedledee {
namespace quil {

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

		void add_qubit(std::string_view qubit_id)
		{
			program_->qubits.emplace_back(qubit_id);
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

	std::vector<std::string> qubits;

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

} // namespace quil
} // namespace tweedledee
