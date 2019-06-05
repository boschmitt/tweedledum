/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../base/source_manager.hpp"
#include "ast/ast.hpp"
#include "preprocessor.hpp"
#include "semantic.hpp"
#include "spdlog/spdlog.h"
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
	// std::shared_ptr<spdlog::logger> logger_;

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
			std::cerr << "Error: expected " << token_name(expected) << " but got "
			          << current_token_.name() << '\n';
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
	parser(preprocessor& pp_lexer, semantic& semantic, source_manager& source_manager)
	    : pp_lexer_(pp_lexer)
	    , semantic_(semantic)
	    , source_manager_(source_manager)
	{
		// logger_ = spdlog::get("quil_logger");
	}

	// Parse
	bool parse()
	{
		consume_token();
		while (not error_) {
			if (current_token_.is(token_kinds::eof)) {
				break;
			}
			// spdlog::info(("[Quil] Current token!!");
			switch (current_token_.kind) {
			// These new line are annoying, are they _really_
			// necessary ?
			case token_kinds::new_line:
				consume_token();
				break;

			case token_kinds::kw_defcircuit:
				semantic_.on_circuit_definition(parse_circuit_declaration());
				break;

			case token_kinds::kw_defgate:
				semantic_.on_gate_definition(parse_gate_declaration());
				break;

			case token_kinds::identifier:
				semantic_.on_gate_statement(parse_gate_statement());
				break;

			default:
				spdlog::info("[Quil] Error; current token: {} {}!!",
				              current_token_.name(),
				              source_manager_.location_str(current_token_.location));
				// std::cout << "Error\n";
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
	/*! \brief Parse circuit declaration

	  Sometimes it is convenient to name and parameterize a particular
	  sequence of Quil instructions for use as a subroutine to other quantum
	  programs. It requires a list of formal arguments which can be
	  substituted with either classical addresses or qubits.

	  Similar to parametric gates, circuint can optionally specify a list of
	  parameters, specified as a comma-separated list in parentheses following
	  the circuit name.

	*/
	// DEFCIRCUIT name (LPAREN param (COMMA param)* RPAREN)? qubitVariable* COLON NEWLINE circuit
	std::unique_ptr<decl_circuit> parse_circuit_declaration()
	{
		// If we get here, then 'DEFCIRCUIT' was matched
		// std::cout << "Circuit declaration\n";
		consume_token();
		auto name = expect_and_consume_token(token_kinds::identifier);
		auto definition = decl_circuit::builder(name.location, name);

		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				parse_parameter_list(definition);
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		do {
			auto qubit_id = expect_and_consume_token(token_kinds::identifier);
			auto argument = decl_argument::build(qubit_id.location, qubit_id);
			semantic_.on_argument_declaration(argument.get());
			definition.add_child(std::move(argument));
			if (try_and_consume_token(token_kinds::colon)) {
				break;
			}
		} while (1);
		expect_and_consume_token(token_kinds::new_line);
		parse_circuit_body(definition);
		semantic_.clear_scope();
		if (not error_) {
			return definition.finish();
		}
		std::cout << "Circuit definition error\n";
		return nullptr;
	}

	/*! \brief Parse circuit declaration body
	*/
	bool parse_circuit_body(decl_circuit::builder& builder)
	{
		do {
			if (not try_and_consume_token(token_kinds::tab)) {
				break;
			}
			switch (current_token_.kind) {
			case token_kinds::identifier:
			 	builder.add_child(parse_gate_statement(true));
			 	break;

			default:
				goto end;
			}
		} while (1);
	end:
		return true;
	}

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
	// DEFGATE name (LPAREN param (COMMA param)* RPAREN)? COLON NEWLINE matrix ;
	std::unique_ptr<decl_gate> parse_gate_declaration()
	{
		// If we get here, then 'DEFGATE' was matched
		// std::cout << "Gate declaration\n";
		consume_token();
		auto name = expect_and_consume_token(token_kinds::identifier);
		auto declaration = decl_gate::builder(name.location, name);

		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				parse_parameter_list(declaration);
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		expect_and_consume_token(token_kinds::colon);
		expect_and_consume_token(token_kinds::new_line);
		parse_matrix(declaration);
		semantic_.clear_scope();
		if (not error_) {
			return declaration.finish();
		}
		return nullptr;
	}

	/*! \brief Parse a list of parameters

	    Formal parameters are names prepended with a ‘%’ symbol, which can
	    be defined in a gate and circuit declarations.

	*/
	// PERCENTAGE IDENTIFIER (COMMA PERCENTAGE IDENTIFIER)*
	template<typename Builder>
	bool parse_parameter_list(Builder& builder)
	{
		do {
			expect_and_consume_token(token_kinds::percentage);
			auto identifier = expect_and_consume_token(token_kinds::identifier);
			auto param = decl_parameter::build(identifier.location, identifier);
			semantic_.on_parameter_declaration(param.get());
			builder.add_child(std::move(param));
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return true;
	}

	/*! \brief Parse the matrix definition of a gate
	*/
	// TAB expression (COMMA expression)* NEWLINE)* TAB expression (COMMA expression)*
	void parse_matrix(decl_gate::builder& builder)
	{
		spdlog::info("[Quil] Parsing matrix");
		auto matrix = matrix::builder(current_token_.location);
		do {
			if (not try_and_consume_token(token_kinds::tab)) {
				break;
			}
			if (try_and_consume_token(token_kinds::new_line)) {
				break;
			}
			auto row = matrix_row::builder(current_token_.location);
			parse_expression_list(row);
			expect_and_consume_token(token_kinds::new_line);
			matrix.add_child(row.finish());
		} while (1);
		builder.add_child(matrix.finish());
		return;
	}

	/*! \brief Parse gate statement (aka gate invocation)
	 */
	// name (LPAREN param (COMMA param)* RPAREN)? qubit+ ;
	std::unique_ptr<stmt_gate> parse_gate_statement(bool inside_def_circuit = false)
	{
		// If we get here, then an identifier was matched
		auto identifier = expect_and_consume_token(token_kinds::identifier);
		auto stmt_builder = stmt_gate::builder(identifier.location, identifier);
		auto gate_reference = semantic_.create_declaration_reference(identifier.location, identifier);

		stmt_builder.add_child(std::move(gate_reference));
		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				parse_expression_list(stmt_builder);
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		if (inside_def_circuit) {
			do {
				auto qubit_id = expect_and_consume_token(token_kinds::identifier);
				auto qubit_reference = semantic_.create_declaration_reference(qubit_id.location, qubit_id);
				stmt_builder.add_child(std::move(qubit_reference));
				if (current_token_.kind == token_kinds::new_line) {
					break;
				}
			} while (1);
		} else {
			do {
				auto qubit_id = expect_and_consume_token(token_kinds::integer);
				auto qubit = qubit::build(qubit_id.location, qubit_id);
				stmt_builder.add_child(std::move(qubit));
				if (current_token_.kind == token_kinds::new_line) {
					break;
				}
			} while (1);
		}
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
			auto sign = expr_sign::builder(current_token_.location, '-');
			auto atom = parse_expression();
			sign.add_child(std::move(atom));
			return sign.finish();
		}

		std::unique_ptr<ast_node> atom = nullptr;
		bool is_imaginary;
		unary_ops unary_op = unary_ops::unknown;
		switch (current_token_.kind) {
		// function LPAREN expression RPAREN 
		// function : SIN | COS | SQRT | EXP | CIS ;
		case token_kinds::kw_uop_sin:
			unary_op = unary_ops::sin;
			goto lbl_unary_op;

		case token_kinds::kw_uop_cos:
			unary_op = unary_ops::cos;
			goto lbl_unary_op;

		case token_kinds::kw_uop_cis:
			unary_op = unary_ops::cis;
			goto lbl_unary_op;

		case token_kinds::kw_uop_exp:
			unary_op = unary_ops::exp;
			goto lbl_unary_op;

		case token_kinds::kw_uop_sqrt:
			unary_op = unary_ops::sqrt;
			goto lbl_unary_op;

		case token_kinds::integer:
			is_imaginary = try_and_consume_token(token_kinds::kw_i);
			atom = expr_integer::build(current_token_.location, current_token_, is_imaginary);
			break;

		case token_kinds::real:
			is_imaginary = try_and_consume_token(token_kinds::kw_i);
			atom = expr_real::build(current_token_.location, current_token_, is_imaginary);
			break;

		case token_kinds::kw_pi:
			is_imaginary = try_and_consume_token(token_kinds::kw_i);
			atom = expr_real::build(current_token_.location, M_PI, is_imaginary);
			break;

		case token_kinds::kw_i:
			atom =  expr_real::build(current_token_.location, 1, true);
			break;

		// variable
		case token_kinds::percentage:
			consume_token();
			atom = semantic_.create_declaration_reference(current_token_.location, current_token_);
			break;

		default:
			break;
		}
		consume_token();
		return atom;

	lbl_unary_op:
		consume_token();
		auto unary_op_atom = expr_unary_op::builder(current_token_.location, unary_op);
		expect_and_consume_token(token_kinds::l_paren);
		atom = parse_expression();
		expect_and_consume_token(token_kinds::r_paren);
		unary_op_atom.add_child(std::move(atom));
		return unary_op_atom.finish();
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
			auto binary_op = expr_binary_op::builder(current_token_.location, op);
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
			// This is a hack to add the evaluated expression, so instead of '2+2' it
			// adds '4' to the AST
			// auto evaluated_expr = expr_real::build(expr->location(), evaluate(*expr));
			builder.add_child(std::move(expr));
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
			switch (static_cast<const expr_binary_op&>(node).op()) {
			case '+':
				return evaluate(*static_cast<const expr_binary_op&>(node).begin())
				       + evaluate(*static_cast<const expr_binary_op&>(node).back());
			case '-':
				return evaluate(*static_cast<const expr_binary_op&>(node).begin())
				       - evaluate(*static_cast<const expr_binary_op&>(node).back());
			case '*':
				return evaluate(*static_cast<const expr_binary_op&>(node).begin())
				       * evaluate(*static_cast<const expr_binary_op&>(node).back());
			case '/':
				return evaluate(*static_cast<const expr_binary_op&>(node).begin())
				       / evaluate(*static_cast<const expr_binary_op&>(node).back());
			default:
				return 0.0;
			}
			// return handle_container<expr_binary_op>(node, cb, functor);

		case ast_node_kinds::expr_sign:
			return -(evaluate(*static_cast<const expr_sign&>(node).begin()));

		case ast_node_kinds::expr_integer:
		case ast_node_kinds::expr_real:
			return static_cast<const expr_real&>(node).evaluate();

		default:
			break;
		}
		return 0.0;
	}
};

} // namespace quil
} // namespace tweedledee
