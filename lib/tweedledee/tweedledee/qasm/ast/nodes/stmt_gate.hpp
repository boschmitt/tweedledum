/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "expr_decl_ref.hpp"
#include "../ast_context.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>

namespace tweedledee {
namespace qasm {

// A `stmt_gate` node has three children, one of which optional.
// The children objects are in order:
//
// * A `expr_decl_ref *` which is a reference for the gate declaration.
//    Always present.
//
// * A "list_exps *" for the parameters list. 
//    Optional. (Not all gates are parametrizable)
//
// * A "list_any *" for the qubit argument list. (At least one qubit argument is required)
//    Always present.
class stmt_gate
    : public ast_node
    , public ast_node_container<stmt_gate, ast_node> {
private:
	//Configure bits
	enum {
		has_params_ = 0
	};

public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location)
		    : statement_(new (*ctx) stmt_gate(location))
		{}

		void add_gate_decl(ast_node* child)
		{
			statement_->add_child(child);
		}

		void add_parameters(ast_node* parameters)
		{
			statement_->config_bits_ |= (1 << has_params_);
			statement_->add_child(parameters);
		}

		void add_arguments(ast_node* arguments)
		{
			statement_->add_child(arguments);
		}

		stmt_gate* finish()
		{
			return statement_;
		}

	private:
		stmt_gate* statement_;
	};

	bool has_parameters() const
	{
		return ((this->config_bits_ >> has_params_) & 1) == 1;
	}

	ast_node* gate()
	{
		ast_node const* gate = &(*(this->begin()));
		return static_cast<expr_decl_ref const*>(gate)->declaration();
	}

	ast_node* parameters()
	{
		auto iter = this->begin();
		if (has_parameters()) {
			++iter;
			return &(*iter);
		}
		return nullptr;
	}

	// FIXME: this is hacky, implement random access iterator
	ast_node* arguments()
	{
		auto iter = this->begin();
		++iter;
		if (has_parameters()) {
			++iter;
			return &(*iter);
		}
		return &(*iter);
	}

private:
	stmt_gate(uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::stmt_gate;
	}
};

} // namespace qasm
} // namespace tweedledee
