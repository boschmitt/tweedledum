/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../base/source_manager.hpp"
#include "ast/ast.hpp"
#include "preprocessor.hpp"
#include "semantic.hpp"
#include "token.hpp"
#include "token_kinds.hpp"

#include <cmath>
#include <cstdint>
#include <memory>

namespace tweedledee {
namespace quil {

// This implements a parser for Quil. After parsing units of the
// grammar, productions are invoked to handle whatever has been read.
//
// TODO: error recovery.
class parser {
	preprocessor& pp_lexer_;
	semantic& semantic_;
	source_manager& source_manager_;

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
			std::cerr << "Error: expected " << token_name(expected)
			          << " but got " << current_token_.name()
			          << '\n';
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
	parser(preprocessor& pp_lexer, semantic& semantic,
	       source_manager& source_manager)
	    : pp_lexer_(pp_lexer)
	    , semantic_(semantic)
	    , source_manager_(source_manager)
	{}

	// Parse
	bool parse()
	{
		consume_token();
		while (not error_) {
			if (current_token_.is(token_kinds::eof)) {
				break;
			}
			switch (current_token_.kind) {
			// These new line are annoying, are they _really_
			// necessary ?
			case token_kinds::new_line:
				consume_token();
				break;

			case token_kinds::identifier:
				semantic_.on_gate_statement(
				    parse_gate_statement());
				break;

			default:
				std::cout << "Error\n";
				error_ = true;
				break;
			}
		}
		if (error_) {
			return false;
		}
		return true;
	}

private:
	/*! \brief Parse gate statement (aka gate invocation)
	 */
	// name (LPAREN param (COMMA param)* RPAREN)? qubit+ ;
	std::unique_ptr<stmt_gate> parse_gate_statement()
	{
		// If we get here, then an identifier was matched
		auto identifier
		    = expect_and_consume_token(token_kinds::identifier);
		auto stmt_builder
		    = stmt_gate::builder(identifier.location, identifier);

		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				parse_expression_list(stmt_builder);
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		do {
			auto qubit_id = expect_and_consume_token(token_kinds::integer);
			semantic_.on_qubit(qubit_id);
			auto qubit = qubit::build(qubit_id.location, qubit_id);
			stmt_builder.add_child(std::move(qubit));
			if (current_token_.kind == token_kinds::new_line) {
				break;
			}
		} while (1);
		expect_and_consume_token(token_kinds::new_line);
		if (not error_) {
			return stmt_builder.finish();
		}
		return nullptr;
	}

	/*! \brief Parse expression atom
	 */
	std::unique_ptr<ast_node> parse_expression_atom()
	{
		// LPAREN expression RPAREN
		if (try_and_consume_token(token_kinds::l_paren)) {
			auto atom = parse_expression();
			expect_and_consume_token(token_kinds::r_paren);
			return atom;
		}
		// sign : PLUS | MINUS ;
		if (try_and_consume_token(token_kinds::minus)) {
			auto sign
			    = expr_sign::builder(current_token_.location, '-');
			auto atom = parse_expression();
			sign.add_child(std::move(atom));
			return sign.finish();
		}

		std::unique_ptr<ast_node> atom = nullptr;
		switch (current_token_.kind) {
			// Hack
		case token_kinds::integer:
			atom = expr_real::build(current_token_.location,
			                           current_token_);
			break;

		case token_kinds::real:
			atom = expr_real::build(current_token_.location,
			                        current_token_);
			break;

		case token_kinds::kw_pi:
			atom = expr_real::build(current_token_.location, M_PI);
			break;

		default:
			break;
		}
		consume_token();
		return atom;
	}

	/*! \brief Parse expression
	 */
	std::unique_ptr<ast_node> parse_expression(unsigned min_precedence = 1)
	{
		auto atom_lhs = parse_expression_atom();
		while (1) {
			auto next_min_precedence = min_precedence;
			auto op = '?';

			switch (current_token_.kind) {
			case token_kinds::plus:
				if (min_precedence > 1)
					goto end;
				op = '+';
				next_min_precedence = 2;
				break;

			case token_kinds::minus:
				if (min_precedence > 1)
					goto end;
				op = '+';
				next_min_precedence = 2;
				break;

			case token_kinds::star:
				if (min_precedence > 2)
					goto end;
				op = '*';
				next_min_precedence = 3;
				break;

			case token_kinds::slash:
				if (min_precedence > 2)
					goto end;
				op = '/';
				next_min_precedence = 3;
				break;

			case token_kinds::caret:
				break;
			default:
				goto end;
			}

			consume_token();
			auto atom_rhs = parse_expression(next_min_precedence);
			auto binary_op = expr_binary_op::builder(
			    current_token_.location, op);
			binary_op.add_child(std::move(atom_lhs));
			binary_op.add_child(std::move(atom_rhs));
			atom_lhs = binary_op.finish();
		}
	end:
		return atom_lhs;
	}

	/*! \brief Parse expression list
	 */
	template<typename Builder>
	void parse_expression_list(Builder& builder)
	{
		do {
			auto expr = parse_expression();
			// This is a hack to add the evaluated expression, so instead of '2+2' it adds '4'
			// to the AST
			auto evaluated_expr = expr_real::build(expr->location(), evaluate(*expr));
			builder.add_child(std::move(evaluated_expr));
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return;
	}

	float evaluate(const ast_node& node)
	{
		switch (node.kind()) {
		case ast_node_kinds::expr_binary_op:
			switch (static_cast<const expr_binary_op &>(node).op()) {
			case '+':
				return evaluate(*static_cast<const expr_binary_op &>(node).begin()) + evaluate(*static_cast<const expr_binary_op &>(node).back());
			case '-':
				return evaluate(*static_cast<const expr_binary_op &>(node).begin()) - evaluate(*static_cast<const expr_binary_op &>(node).back());
			case '*':
				return evaluate(*static_cast<const expr_binary_op &>(node).begin()) * evaluate(*static_cast<const expr_binary_op &>(node).back());
			case '/':
				return evaluate(*static_cast<const expr_binary_op &>(node).begin()) / evaluate(*static_cast<const expr_binary_op &>(node).back());
			default:
				return 0.0;
			}
			// return handle_container<expr_binary_op>(node, cb, functor);

		case ast_node_kinds::expr_sign:
			return -(evaluate(*static_cast<const expr_sign &>(node).begin()));

		case ast_node_kinds::expr_integer:
		case ast_node_kinds::expr_real:
			return static_cast<const expr_real &>(node).evaluate();

		default:
			break;
		}
		return 0.0;
	}
};

} // namespace quil
} // namespace tweedledee
