/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../base/source_manager.hpp"
#include "../base/diagnostic.hpp"
#include "ast/ast.hpp"
#include "preprocessor.hpp"
#include "token.hpp"
#include "token_kinds.hpp"

#include <fmt/format.h>

namespace tweedledee {
namespace quil {

// This implements a parser for Open QASM 2.0. After parsing units of the
// grammar, productions are invoked to handle whatever has been read.
//
// TODO: error recovery.
class parser {
	preprocessor& pp_lexer_;
	source_manager& source_manager_;
	diagnostic_engine& diagnostic_;
	std::unique_ptr<ast_context> context_;

	bool error_ = false;

	// The current token we are peeking.
	token current_token_;

	// The location of the token we previously consumed. This is used
	// for diagnostics in which we expected to see a token following
	// another token (e.g., the ';' at the end of a statement).
	unsigned prev_token_location_;

private:
	// Consume the current token 'current_token_' and lex the next one.
	// Returns the location of the consumed token.
	uint32_t consume_token()
	{
		prev_token_location_ = current_token_.location;
		current_token_ = pp_lexer_.next_token();
		return prev_token_location_;
	}

	// The parser expects that the current token is of 'expected' kind.
	// If it is not, it emits a diagnostic, puts the parser in a error
	// state and returns the current_token_. Otherwise consumes the token
	// and returns it.
	token expect_and_consume_token(token_kinds expected)
	{
		if (error_) {
			return current_token_;
		}
		if (current_token_.is_not(expected)) {
			diagnostic_.report(diagnostic_levels::error,
			                   source_manager_.location_str(current_token_.location),
			                   fmt::format("expected {} but got {}",
			                               token_name(expected), current_token_.name()));
			error_ = true;
			return current_token_;
		}
		auto return_token = current_token_;
		consume_token();
		return return_token;
	}

	// The parser try to see if the current token is of 'expected' kind.
	// If it is not, returns false. Otherwise consumes the token and
	// returns true.
	bool try_and_consume_token(token_kinds expected)
	{
		if (current_token_.is_not(expected) || error_) {
			return false;
		}
		consume_token();
		return true;
	}

public:
	parser(preprocessor& pp_lexer, source_manager& source_manager, diagnostic_engine& diagnostic)
	    : pp_lexer_(pp_lexer)
	    , source_manager_(source_manager)
	    , diagnostic_(diagnostic)
	    , context_(std::make_unique<ast_context>(source_manager, diagnostic))
	{}

	// Parse
	std::unique_ptr<ast_context> parse()
	{
		consume_token();
		while (not error_) {
			if (current_token_.is(token_kinds::eof)) {
				break;
			}
			switch (current_token_.kind) {
			// These new line are annoying, are they _really_ necessary ?
			case token_kinds::new_line:
				consume_token();
				break;

			case token_kinds::kw_defgate:
				context_->add_node(parse_defgate());
				break;

			default:
				error_ = true;
				break;
			}
			if (diagnostic_.num_errors) {
				error_ = true;
			}
		}
		if (error_) {
			return nullptr;
		}
		return std::move(context_);
	}

private:
	/*! \brief Parse gate declaration

	  In Quil, every gate is defined separately from its invocation.
	  There are two gate-related concepts in Quil: static and parametric gates.
	  A static gate is an operator in U(2Nq ), and a parametric gate is a function
	  Cn -> U(2Nq).

	  Static gates are defined by their real or complex matrix entries.
	  The gate is declared using the DEFGATE directive followed by
	  comma-separated lists of matrix entries indented by exactly four spaces.

	  Parametric gates are the same, except for the allowance of formal
	  parameters, which are names prepended with a '%' symbol.
	  Comma-separated formal parameters are listed in parentheses following
	  the gate name, as is usual.

	*/
	// DEFGATE name (LPAREN param (COMMA param)* RPAREN)? COLON NEWLINE matrix
	ast_node* parse_defgate()
	{
		// If we get here, then 'DEFGATE' was matched
		// std::cout << "Gate declaration\n";
		consume_token();
		auto name = expect_and_consume_token(token_kinds::identifier);
		auto decl = decl_gate::builder(context_.get(), name.location, name);

		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				decl.add_parameters(parse_idlist());
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		expect_and_consume_token(token_kinds::colon);
		expect_and_consume_token(token_kinds::new_line);
		decl.add_matrix(parse_matrix());
		context_->clear_scope();
		if (!error_) {
			return decl.finish();
		}
		return nullptr;
	}

	/*! \brief Parse the matrix definition of a gate */
	// TAB expression (COMMA expression)* NEWLINE)* TAB expression (COMMA expression)*
	ast_node* parse_matrix()
	{
		auto matrix_builder = decl_matrix::builder(context_.get(), current_token_.location);
		do {
			if (!try_and_consume_token(token_kinds::tab)) {
				break;
			}
			if (try_and_consume_token(token_kinds::new_line)) {
				break;
			}
			matrix_builder.add_row(parse_explist());
			expect_and_consume_token(token_kinds::new_line);
		} while (1);
		return matrix_builder.finish();
	}
	
	ast_node* parse_explist()
	{
		auto builder = list_exps::builder(context_.get(), current_token_.location);
		do {
			auto expr = parse_exp();
			builder.add_child(expr);
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return builder.finish();
	}

	ast_node* parse_exp(unsigned min_precedence = 1)
	{
		ast_node* atom_lhs = parse_atom();
		while (1) {
			auto next_min_precedence = min_precedence;
			binary_ops op = binary_ops::unknown;

			switch (current_token_.kind) {
			case token_kinds::plus:
				if (min_precedence > 1) {
					goto end;
				}
				op = binary_ops::addition;
				next_min_precedence = 2;
				break;

			case token_kinds::minus:
				if (min_precedence > 1) {
					goto end;
				}
				op = binary_ops::subtraction;
				next_min_precedence = 2;
				break;

			case token_kinds::star:
				if (min_precedence > 2) {
					goto end;
				}
				op = binary_ops::multiplication;
				next_min_precedence = 3;
				break;

			case token_kinds::slash:
				if (min_precedence > 2) {
					goto end;
				}
				op = binary_ops::division;
				next_min_precedence = 3;
				break;

			case token_kinds::caret:
				if (min_precedence > 3) {
					goto end;
				}
				op = binary_ops::exponentiation;
				next_min_precedence = 4;
				break;

			default:
				goto end;
			}

			consume_token();
			ast_node* atom_rhs = parse_exp(next_min_precedence);
			auto binary_op = expr_binary_op::builder(context_.get(), current_token_.location, op);
			binary_op.add_child(atom_lhs);
			binary_op.add_child(atom_rhs);
			atom_lhs = binary_op.finish();
		}
	end:
		return atom_lhs;
	}

#pragma region Helper functions
	ast_node* parse_atom()
	{
		if (try_and_consume_token(token_kinds::l_paren)) {
			auto atom = parse_exp();
			expect_and_consume_token(token_kinds::r_paren);
			return atom;
		}
		if (try_and_consume_token(token_kinds::minus)) {
			auto sign = expr_unary_op::builder(context_.get(), current_token_.location, unary_ops::minus);
			auto atom = parse_exp();
			sign.add_child(atom);
			return sign.finish();
		}

		ast_node* atom = nullptr;
		switch (current_token_.kind) {
		case token_kinds::identifier:
			atom = create_decl_reference(current_token_.location, current_token_);
			break;

		case token_kinds::integer:
			atom = expr_integer::create(context_.get(), current_token_.location, current_token_);
			break;

		case token_kinds::kw_pi:
			atom = expr_pi::create(context_.get(), current_token_.location);
			break;

		case token_kinds::real:
			atom = expr_real::create(context_.get(), current_token_.location, current_token_);
			break;
		default:
			break;
		}
		if (atom != nullptr) {
			consume_token();
			return atom;
		}

		auto op = unary_ops::unknown;
		switch (current_token_.kind) {
		case token_kinds::kw_uop_sin:
			op = unary_ops::sin;
			break;

		case token_kinds::kw_uop_cos:
			op = unary_ops::cos;
			break;

		case token_kinds::kw_uop_exp:
			op = unary_ops::exp;
			break;

		case token_kinds::kw_uop_sqrt:
			op = unary_ops::sqrt;
			break;

		default:
			std::cout << "Error\n";
			return nullptr;
		}

		consume_token();
		auto unary_op = expr_unary_op::builder(context_.get(), current_token_.location, op);
		expect_and_consume_token(token_kinds::l_paren);
		atom = parse_exp();
		expect_and_consume_token(token_kinds::r_paren);
		unary_op.add_child(atom);
		return unary_op.finish();
	}

	/*! \brief Parse a identifier */
	list_ids* parse_idlist()
	{
		auto builder = list_ids::builder(context_.get(), current_token_.location);
		do {
			auto identifier = expect_and_consume_token(token_kinds::identifier);
			auto param = decl_param::build(context_.get(), identifier.location, identifier);
			context_->add_decl_parameter(identifier, param);
			builder.add_child(param);
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return builder.finish();
	}

	expr_decl_ref* create_decl_reference(uint32_t location, std::string_view identifier)
	{
		auto declaration = context_->find_declaration(identifier);
		if (declaration) {
			return expr_decl_ref::build(context_.get(), location, declaration);
		}
		diagnostic_.report(diagnostic_levels::error, source_manager_.location_str(location),
		                   fmt::format("undefined reference to {}", identifier));
		return nullptr;
	}
#pragma endregion
};

} // namespace quil
} // namespace tweedledee
