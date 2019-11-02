/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_context.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>

namespace tweedledee {
namespace qasm {

// A `decl_gate` node has three childs, one of which optional.
// The children objects are in order:
//
// * A `list_ids *` for the parameter identifier list.
//    Present if and only if has_parameters().
//
// * A "list_ids *" for the qubit argument identifier list. (At least one qubit argument is required)
//    Always present.
//
// * A "list_gops *" for the body.
//    Always present.
class decl_gate
    : public ast_node
    , public ast_node_container<decl_gate, ast_node> {
private:
	//Configure bits
	enum {
		has_params_ = 0
	};

public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location, std::string_view identifier)
		    : statement_(new (*ctx) decl_gate(location, identifier))
		{}

		void add_parameters(ast_node* parameters)
		{
			statement_->config_bits_ |= (1 << has_params_);
			statement_->add_child(parameters);
		}

		void add_arguments(ast_node* arguments)
		{
			statement_->add_child(arguments);
		}

		void add_body(ast_node* ops)
		{
			statement_->add_child(ops);
		}

		decl_gate* finish()
		{
			return statement_;
		}

	private:
		decl_gate* statement_;
	};

	std::string_view identifier() const
	{
		return identifier_;
	}

	bool has_parameters() const
	{
		return ((this->config_bits_ >> has_params_) & 1) == 1;
	}

	ast_node* parameters()
	{
		if (has_parameters()) {
			return &(*(this->begin()));
		}
		return nullptr;
	}

	// FIXME: this is hacky, implement random access iterator?
	ast_node* arguments()
	{
		auto iter = this->begin();
		if (has_parameters()) {
			++iter;
			return &(*iter);
		}
		return &(*iter);
	}

	ast_node* body()
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
	decl_gate(uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(identifier)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_gate;
	}

private:
	std::string identifier_;
};

} // namespace qasm
} // namespace tweedledee
