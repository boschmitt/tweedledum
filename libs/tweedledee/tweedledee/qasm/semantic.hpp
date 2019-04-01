/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../base/diagnostic.hpp"
#include "../base/source_manager.hpp"
#include "ast/ast.hpp"

#include <fmt/format.h>

namespace tweedledee {
namespace qasm {

// This implements semantic analysis and AST building.
class semantic {
	program::builder program_;

	source_manager& source_manager_;
	diagnostic_engine& diagnostic_;

	std::unordered_map<std::string_view, ast_node*> identifier_table_;
	std::unordered_map<std::string_view, decl_param*> scope_;

public:
	semantic(source_manager& source_manager, diagnostic_engine& diagnostic)
	    : source_manager_(source_manager)
	    , diagnostic_(diagnostic)
	{}

	semantic(const semantic&) = delete;
	void operator=(const semantic&) = delete;

	void clear_scope()
	{
		scope_.clear();
	}

	std::unique_ptr<expr_decl_ref> create_declaration_reference(uint32_t location,
	                                                            std::string_view identifier)
	{
		auto declaration = find_declaration(identifier);
		if (declaration == nullptr) {
			diagnostic_.report(diagnostic_levels::error,
			                   source_manager_.location_str(location),
			                   fmt::format("undefined reference to {}", identifier));
			// return nullptr;
		}
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
		bool ok = scope_.insert({param_decl->identifier(), param_decl}).second;
		if (!ok) {
			diagnostic_.report(diagnostic_levels::error,
			                   source_manager_.location_str(param_decl->location()),
			                   fmt::format("redefinition of {}", param_decl->identifier()));
		}
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
		auto param = scope_.find(identifier);
		if (param != scope_.end()) {
			return param->second;
		}
		auto entry = identifier_table_.find(identifier);
		if (entry != identifier_table_.end()) {
			return entry->second;
		}
		return nullptr;
	}
};

} // namespace qasm
} // namespace tweedledee
