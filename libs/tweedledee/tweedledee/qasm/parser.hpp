/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../base/source_manager.hpp"
#include "ast/ast.hpp"
#include "preprocessor.hpp"
#include "semantic.hpp"
#include "token.hpp"
#include "token_kinds.hpp"

namespace tweedledee {
namespace qasm {

// This implements a parser for Open QASM 2.0. After parsing units of the
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
	std::uint32_t consume_token()
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
			std::cerr << source_manager_.location_str(current_token_.location)
			          << " error: expected " << token_name(expected) << " but got "
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
	{}

	// Parse
	bool parse()
	{
		consume_token();
		parse_header();
		while (not error_) {
			if (current_token_.is(token_kinds::eof)) {
				break;
			}
			switch (current_token_.kind) {
			case token_kinds::kw_creg:
				semantic_.on_register_declaration(
				    parse_register_declaration(RegType::classical));
				break;

			case token_kinds::kw_qreg:
				semantic_.on_register_declaration(
				    parse_register_declaration(RegType::quantum));
				break;

			case token_kinds::kw_cx:
				semantic_.on_cnot(parse_cnot());
				break;

			case token_kinds::kw_gate:
				semantic_.on_gate_declaration(parse_gate_declaration());
				break;

			case token_kinds::kw_u:
				semantic_.on_unitary(parse_unitary());
				break;

			case token_kinds::identifier:
				semantic_.on_gate_statement(parse_gate_statement());
				break;

			default:
				error_ = true;
				break;
			}
		}
		if (error_)
			return false;
		return true;
	}

private:
	// The first (non-comment) line of an Open QASM program must be
	// OPENQASM M.m; indicating a major version M and minor version m.
	void parse_header()
	{
		expect_and_consume_token(token_kinds::kw_openqasm);
		expect_and_consume_token(token_kinds::real);
		expect_and_consume_token(token_kinds::semicolon);
	}

	// <argument> = <id>
	//            | <id> [ <nninteger> ]
	//
	std::unique_ptr<ast_node> parse_argument()
	{
		auto location = current_token_.location;
		auto identifier = expect_and_consume_token(token_kinds::identifier);
		auto declaration_reference = semantic_.create_declaration_reference(location,
		                                                                    identifier);
		if (not try_and_consume_token(token_kinds::l_square)) {
			return declaration_reference;
		}

		auto indexed_reference = expr_reg_idx_ref::builder(location);
		auto idx = expect_and_consume_token(token_kinds::nninteger);
		auto index = expr_integer::build(idx.location, idx);
		expect_and_consume_token(token_kinds::r_square);
		if (not error_) {
			indexed_reference.add_child(std::move(declaration_reference));
			indexed_reference.add_child(std::move(index));
			return indexed_reference.finish();
		}
		return nullptr;
	}

	void parse_argument_list(stmt_gate::builder& builder)
	{
		do {
			auto arg = parse_argument();
			builder.add_child(std::move(arg));
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return;
	}

	//
	//
	std::unique_ptr<ast_node> parse_atom()
	{
		if (try_and_consume_token(token_kinds::l_paren)) {
			auto atom = parse_expression();
			expect_and_consume_token(token_kinds::r_paren);
			return atom;
		}
		if (try_and_consume_token(token_kinds::minus)) {
			auto sign = expr_unary_op::builder(current_token_.location, unary_ops::minus);
			auto atom = parse_expression();
			sign.add_child(std::move(atom));
			return sign.finish();
		}

		std::unique_ptr<ast_node> atom = nullptr;
		switch (current_token_.kind) {
		case token_kinds::identifier:
			atom = semantic_.create_declaration_reference(current_token_.location,
			                                              current_token_);
			break;

		case token_kinds::nninteger:
			atom = expr_integer::build(current_token_.location, current_token_);
			break;

		case token_kinds::kw_pi:
			atom = expr_real::build(current_token_.location, 3.14);
			break;

		case token_kinds::real:
			atom = expr_real::build(current_token_.location, current_token_);
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
		case token_kinds::kw_uop_tan:
			op = unary_ops::tan;
			break;
		case token_kinds::kw_uop_exp:
			op = unary_ops::exp;
			break;
		case token_kinds::kw_uop_ln:
			op = unary_ops::ln;
			break;
		case token_kinds::kw_uop_sqrt:
			op = unary_ops::sqrt;
			break;
		default:
			std::cout << "Error\n";
			return nullptr;
		}
		consume_token();
		auto unary_op = expr_unary_op::builder(current_token_.location, op);
		expect_and_consume_token(token_kinds::l_paren);
		atom = parse_expression();
		expect_and_consume_token(token_kinds::r_paren);
		unary_op.add_child(std::move(atom));
		return unary_op.finish();
	}

	//
	//
	std::unique_ptr<ast_node> parse_expression(unsigned min_precedence = 1)
	{
		auto atom_lhs = parse_atom();
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

	//
	void parse_expression_list(stmt_gate::builder& builder)
	{
		do {
			auto expr = parse_expression();
			builder.add_child(std::move(expr));
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return;
	}

	// <cnot> = CX <argument> , <argument> ;
	//
	std::unique_ptr<stmt_cnot> parse_cnot()
	{
		// If we get here 'CX' was matched
		auto location = current_token_.location;
		consume_token();
		auto arg1 = parse_argument();
		expect_and_consume_token(token_kinds::comma);
		auto arg2 = parse_argument();
		expect_and_consume_token(token_kinds::semicolon);
		if (not error_) {
			auto cnot_builder = stmt_cnot::builder(location);
			cnot_builder.add_child(std::move(arg1));
			cnot_builder.add_child(std::move(arg2));
			return cnot_builder.finish();
		}
		return nullptr;
	}

	//
	bool parse_gate_body(decl_gate::builder& builder)
	{
		do {
			switch (current_token_.kind) {
			case token_kinds::kw_cx:
				builder.add_child(parse_cnot());
				break;

			case token_kinds::kw_u:
				builder.add_child(parse_unitary());
				break;

			case token_kinds::identifier:
				builder.add_child(parse_gate_statement());
				break;

			default:
				goto end;
			}
		} while (1);
	end:
		return true;
	}

	// <gatedecl> = gate <id> <idlist> {
	//            | gate <id> ( ) <idlist> {
	//            | gate <id> ( <idlist> ) <idlist> {
	//
	std::unique_ptr<decl_gate> parse_gate_declaration()
	{
		// If we get here, then either 'gate'
		consume_token();
		auto identifier = expect_and_consume_token(token_kinds::identifier);
		auto decl = decl_gate::builder(identifier.location, identifier);

		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				parse_identifier_list(decl);
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		parse_identifier_list(decl);
		expect_and_consume_token(token_kinds::l_brace);
		if (not try_and_consume_token(token_kinds::r_brace)) {
			parse_gate_body(decl);
			expect_and_consume_token(token_kinds::r_brace);
		}
		semantic_.clear_scope();
		if (not error_) {
			return decl.finish();
		}
		return nullptr;
	}

	std::unique_ptr<stmt_gate> parse_gate_statement()
	{
		// If we get here, then an identifier was matched
		auto identifier = expect_and_consume_token(token_kinds::identifier);
		auto stmt_builder = stmt_gate::builder(identifier.location);
		auto gate_reference = semantic_.create_declaration_reference(identifier.location,
		                                                             identifier);

		stmt_builder.add_child(std::move(gate_reference));
		if (try_and_consume_token(token_kinds::l_paren)) {
			if (not try_and_consume_token(token_kinds::r_paren)) {
				parse_expression_list(stmt_builder);
				expect_and_consume_token(token_kinds::r_paren);
			}
		}
		parse_argument_list(stmt_builder);
		expect_and_consume_token(token_kinds::semicolon);
		if (not error_) {
			return stmt_builder.finish();
		}
		return nullptr;
	}

	// <idlist> = <id>
	//          | <idlist> , <id>
	//
	bool parse_identifier_list(decl_gate::builder& builder)
	{
		do {
			auto identifier = expect_and_consume_token(token_kinds::identifier);
			auto param = decl_param::build(identifier.location, identifier);
			semantic_.on_parameter_declaration(param.get());
			builder.add_child(std::move(param));
			if (not try_and_consume_token(token_kinds::comma)) {
				break;
			}
		} while (1);
		return true;
	}

	// <regdecl> = qreg <id> [ <nninteger> ] ;
	//           | creg <id> [ <nninteger> ] ;
	//
	std::unique_ptr<decl_register> parse_register_declaration(RegType type)
	{
		// If we get here, then either 'qreg' or 'creg' was matched
		auto location = current_token_.location;
		consume_token();
		auto identifier = expect_and_consume_token(token_kinds::identifier);
		expect_and_consume_token(token_kinds::l_square);
		auto size = expect_and_consume_token(token_kinds::nninteger);
		expect_and_consume_token(token_kinds::r_square);
		expect_and_consume_token(token_kinds::semicolon);

		if (not error_) {
			return decl_register::build(location, type, identifier, size);
		}
		return nullptr;
	}

	// <U> = U ( <expr>, <expr>, <expr> ) <argument> ;
	//
	std::unique_ptr<stmt_unitary> parse_unitary()
	{
		// If we get here 'U' was matched
		consume_token();
		auto location = current_token_.location;
		expect_and_consume_token(token_kinds::l_paren);
		auto param1 = parse_expression();
		expect_and_consume_token(token_kinds::comma);
		auto param2 = parse_expression();
		expect_and_consume_token(token_kinds::comma);
		auto param3 = parse_expression();
		expect_and_consume_token(token_kinds::r_paren);
		auto target = parse_argument();
		expect_and_consume_token(token_kinds::semicolon);

		if (not error_) {
			auto unitary_builder = stmt_unitary::builder(location);
			unitary_builder.add_child(std::move(param1));
			unitary_builder.add_child(std::move(param2));
			unitary_builder.add_child(std::move(param3));
			unitary_builder.add_child(std::move(target));
			return unitary_builder.finish();
		}
		return nullptr;
	}
};

} // namespace qasm
} // namespace tweedledee
