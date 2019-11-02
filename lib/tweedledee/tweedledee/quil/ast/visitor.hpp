/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "ast.hpp"

namespace tweedledee {
namespace quil {

template<typename Derived>
class visitor_base {
public:
	void visit(ast_context& context)
	{
		visit(context.root());
	}

	void visit(ast_node* node)
	{
		if (node) {
			dispatch_node(node);
		}
	}

protected:
	/* Containers */
	void visit_decl_gate(decl_gate* node)
	{
		for (ast_node& child : *node) {
			visit(&child);
		}
	}

	void visit_decl_matrix(decl_matrix* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_decl_row(decl_row* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_decl_program(decl_program* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_expr_binary_op(expr_binary_op* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_expr_unary_op(expr_unary_op* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_list_exps(list_exps* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_list_ids(list_ids* node)
	{
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	/* Leafs */
	void visit_expr_integer(expr_integer* node)
	{
		(void) node;
	}

	void visit_expr_pi(expr_pi* node)
	{
		(void) node;
	}

	void visit_expr_real(expr_real* node)
	{
		(void) node;
	}

private:
	// Convenience method for CRTP
	Derived& derived()
	{
		return *static_cast<Derived*>(this);
	}

	void dispatch_node(ast_node* node)
	{
		switch (node->kind()) {
		/* Containers */
		case ast_node_kinds::decl_gate:
			derived().visit_decl_gate(static_cast<decl_gate*>(node));
			break;

		case ast_node_kinds::decl_matrix:
			derived().visit_decl_matrix(static_cast<decl_matrix*>(node));
			break;

		case ast_node_kinds::decl_row:
			derived().visit_decl_row(static_cast<decl_row*>(node));
			break;

		case ast_node_kinds::decl_program:
			derived().visit_decl_program(static_cast<decl_program*>(node));
			break;

		case ast_node_kinds::expr_binary_op:
			derived().visit_expr_binary_op(static_cast<expr_binary_op*>(node));
			break;

		case ast_node_kinds::expr_unary_op:
			derived().visit_expr_unary_op(static_cast<expr_unary_op*>(node));
			break;

		case ast_node_kinds::list_exps:
			derived().visit_list_exps(static_cast<list_exps*>(node));
			break;

		case ast_node_kinds::list_ids:
			derived().visit_list_ids(static_cast<list_ids*>(node));
			break;

		/* Leafs */
		case ast_node_kinds::expr_integer:
			derived().visit_expr_integer(static_cast<expr_integer*>(node));
			break;

		case ast_node_kinds::expr_pi:
			derived().visit_expr_pi(static_cast<expr_pi*>(node));
			break;

		case ast_node_kinds::expr_real:
			derived().visit_expr_real(static_cast<expr_real*>(node));
			break;

		default:
			break;
		}
		return;
	}
};

class ast_printer : public visitor_base<ast_printer> {
public:
	ast_printer(std::ostream& os = std::cout)
	: os_(os)
	{}

	/* Containers */
	void visit_decl_gate(decl_gate* node)
	{
		os_ << fmt::format("{}|- decl_gate {}\n", prefix_, node->identifier());
		visit_children(node);
	}

	void visit_decl_matrix(decl_matrix* node)
	{
		os_ << fmt::format("{}|- decl_matrix\n", prefix_);
		visit_children(node);
	}

	void visit_decl_row(decl_row* node)
	{
		os_ << fmt::format("{}|- decl_row\n", prefix_);
		visit_children(node);
	}

	void visit_decl_program(decl_program* node)
	{
		os_ << "AST for :\n";
		for (auto& child : *node) {
			visit(const_cast<ast_node*>(&child));
		}
	}

	void visit_expr_binary_op(expr_binary_op* node)
	{
		os_ << fmt::format("{}|- expr_binary_op ", prefix_);
		switch (node->op()) {
		case binary_ops::addition:
			os_ << "'+'\n";
			break;

		case binary_ops::subtraction:
			os_ << "'-'\n";
			break;

		case binary_ops::division:
			os_ << "'/'\n";
			break;

		case binary_ops::multiplication:
			os_ << "'*'\n";
			break;

		case binary_ops::exponentiation:
			os_ << "'^'\n";
			break;

		case binary_ops::equality:
			os_ << "'=='\n";
			break;

		default:
			os_ << "'unknown'\n";
			break;
		}
		visit_children(node);
	}

	void visit_expr_unary_op(expr_unary_op* node)
	{
		os_ << fmt::format("{}|- expr_unary_op ", prefix_);
		switch (node->op()) {
		case unary_ops::sin:
			os_ << "'sin'\n";
			break;

		case unary_ops::cos:
			os_ << "'cos'\n";
			break;

		case unary_ops::tan:
			os_ << "'tan'\n";
			break;

		case unary_ops::exp:
			os_ << "'exp'\n";
			break;

		case unary_ops::ln:
			os_ << "'ln'\n";
			break;

		case unary_ops::sqrt:
			os_ << "'sqrt'\n";
			break;

		case unary_ops::minus:
			os_ << "'minus'\n";
			break;

		case unary_ops::plus:
			os_ << "'plus'\n";
			break;

		default:
			os_ << "'unknown'\n";
			break;
		}
		visit_children(node);
	}

	void visit_list_exps(list_exps* node)
	{
		os_ << fmt::format("{}|- list_exps ({})\n", prefix_, node->num_children());
		visit_children(node);
	}

	void visit_list_ids(list_ids* node)
	{
		os_ << fmt::format("{}|- list_ids ({})\n", prefix_, node->num_children());
		visit_children(node);
	}

	/* Leafs */
	void visit_decl_param(decl_param* node)
	{
		os_ << fmt::format("{}|- decl_param {}\n", prefix_, node->identifier());
	}

	void visit_expr_integer(expr_integer* node)
	{
		os_ << fmt::format("{}|- expr_integer {}\n", prefix_, node->evaluate());
	}

	void visit_expr_pi(expr_pi* node)
	{
		(void) node;
		os_ << fmt::format("{}|- expr_pi\n", prefix_);
	}

	void visit_expr_real(expr_real* node)
	{
		os_ << fmt::format("{}|- expr_real {}\n", prefix_, node->value());
	}

private:
	template<typename NodeT>
	void visit_children(NodeT* node)
	{
		prefix_ += "| ";
		for (ast_node& child : *node) {
			visit(&child);
		}
		prefix_.pop_back();
		prefix_.pop_back();
	}

private:
	std::string prefix_;
	std::ostream& os_;
};

} // namespace quil
} // namespace tweedledee
