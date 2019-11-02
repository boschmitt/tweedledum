/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_context.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <string>

namespace tweedledee {
namespace quil {

class decl_matrix
    : public ast_node
    , public ast_node_container<decl_matrix, ast_node> {
public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location)
		    : node_(new (*ctx) decl_matrix(location))
		{}

		void add_row(ast_node* child)
		{
			node_->add_child(child);
		}

		decl_matrix& get()
		{
			return *node_;
		}

		decl_matrix* finish()
		{
			return node_;
		}

	private:
		decl_matrix* node_;
	};

private:
	decl_matrix(uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_matrix;
	}
};

class decl_row
    : public ast_node
    , public ast_node_container<decl_row, ast_node> {
public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location)
		    : node_(new (*ctx) decl_row(location))
		{}

		void add_column(ast_node* child)
		{
			node_->add_child(child);
		}

		decl_row& get()
		{
			return *node_;
		}

		decl_row* finish()
		{
			return node_;
		}

	private:
		decl_row* node_;
	};

private:
	decl_row(uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_row;
	}
};


// A `decl_gate` node has three childs, one of which optional.
// The children objects are in order:
//
// * A `list_ids *` for the parameter identifier list.
//    Present if and only if has_parameters().
//
// * A "decl_matrix *" for the body.
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

		void add_matrix(ast_node* decl_matrix)
		{
			statement_->add_child(decl_matrix);
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
	ast_node* matrix()
	{
		auto iter = this->begin();
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

} // namespace quil
} // namespace tweedledee
