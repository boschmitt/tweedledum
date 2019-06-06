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

// A `expr_argument` is a reference to a register (quantum or classical) or to an entry on a register
// (qubit or cbit). The node has two children, one of which optional.
//
// The children objects are in order:
//
// * A `expr_decl_ref *` which is a reference for the register declaration.
//    Always present.
//
// * A "expr_interger *" an index indicating which entry. 
//    Optional.
class expr_argument
	: public ast_node
	, public ast_node_container<expr_argument, ast_node> {
private:
	//Configure bits
	enum {
		has_index_ = 0
	};

public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location)
			: statement_(new (*ctx) expr_argument(location))
		{}

		void add_register_decl(ast_node* child)
		{
			statement_->add_child(child);
		}

		void add_index(ast_node* index)
		{
			statement_->config_bits_ |= (1 << has_index_);
			statement_->add_child(index);
		}

		expr_argument* finish()
		{
			return statement_;
		}

	private:
		expr_argument* statement_;
	};

	bool has_index() const
	{
		return ((this->config_bits_ >> has_index_) & 1) == 1;
	}

	ast_node* register_decl()
	{
		ast_node const* gate = &(*(this->begin()));
		return static_cast<expr_decl_ref const*>(gate)->declaration();
	}

	ast_node* index()
	{
		auto iter = this->begin();
		if (has_index()) {
			++iter;
			return &(*iter);
		}
		return nullptr;
	}

private:
	expr_argument(uint32_t location)
		: ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::expr_argument;
	}
};

} // namespace qasm
} // namespace tweedledee
