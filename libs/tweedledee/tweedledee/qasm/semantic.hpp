/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "ast/ast.hpp"

namespace tweedledee {
namespace qasm {

// This implements semantic analysis and AST building.
class semantic {
	program::builder program_;

	std::unordered_map<std::string_view, ast_node*> identifier_table_;
	std::unordered_map<std::string_view, decl_param*> scope_;

public:
	semantic() = default;
	semantic(const semantic&) = delete;
	void operator=(const semantic&) = delete;

	void clear_scope()
	{
		scope_.clear();
	}

	std::unique_ptr<expr_decl_ref> create_declaration_reference(std::uint32_t location,
	                                                      std::string_view identifier)
	{
		auto declaration = find_declaration(identifier);
		return expr_decl_ref::build(location, declaration);
	}

	void on_cnot(std::unique_ptr<stmt_cnot> stmt)
	{
		if (stmt == nullptr) {
			return;
		}
		program_.add_child(std::move(stmt));
	}

	void on_gate_declaration(std::unique_ptr<decl_gate> gate_decl)
	{
		if (gate_decl == nullptr) {
			return;
		}
		identifier_table_.insert({gate_decl->identifier(), &(*gate_decl)});
		program_.add_child(std::move(gate_decl));
	}

	void on_gate_statement(std::unique_ptr<stmt_gate> gate_stmt)
	{
		if (gate_stmt == nullptr) {
			return;
		}
		program_.add_child(std::move(gate_stmt));
	}

	// FIXME: I wonder is this is the class which should mantain this
	// declaration table.
	void on_parameter_declaration(decl_param* param_decl)
	{
		if (param_decl == nullptr) {
			return;
		}
		scope_.insert({param_decl->identifier(), param_decl});
	}

	void on_register_declaration(std::unique_ptr<decl_register> reg_decl)
	{
		if (reg_decl == nullptr) {
			return;
		}
		identifier_table_.insert({reg_decl->identifier(), &(*reg_decl)});
		program_.add_child(std::move(reg_decl));
	}

	void on_unitary(std::unique_ptr<stmt_unitary> unitary_stmt)
	{
		if (unitary_stmt == nullptr) {
			return;
		}
		program_.add_child(std::move(unitary_stmt));
	}

	std::unique_ptr<program> finish()
	{
		return program_.finish();
	}

private:
	ast_node* find_declaration(std::string_view identifier)
	{
		std::cout << "find: " << identifier << '\n';
		auto param = scope_.find(identifier);
		if (param != scope_.end()) {
			return param->second;
		}
		auto entry = identifier_table_.find(identifier);
		if (entry != identifier_table_.end()) {
			return entry->second;
		}
		std::cout << "not found\n";
		return nullptr;
	}
};

} // namespace qasm
} // namespace tweedledee
