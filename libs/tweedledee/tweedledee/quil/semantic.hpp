/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "ast/ast.hpp"

#include <memory>
#include <string_view>
#include <unordered_set>

namespace tweedledee {
namespace quil {

// This implements semantic analysis and AST building.
class semantic {
	program::builder program_;
	std::unordered_set<std::string_view> qubits_;

	std::unordered_map<std::string_view, ast_node*> identifier_table_;
	std::unordered_map<std::string_view, const ast_node*> scope_;

public:
	semantic() = default;
	semantic(const semantic&) = delete;
	void operator=(const semantic&) = delete;

	void clear_scope()
	{
		std::cout << "cleaning scope\n";
		scope_.clear();
	}

	std::unique_ptr<stmt_decl_reference>
	create_declaration_reference(std::uint32_t location,
	                             std::string_view identifier)
	{
		auto declaration = find_declaration(identifier);
		return stmt_decl_reference::build(location, declaration);
	}

	void on_circuit_definition(std::unique_ptr<decl_circuit> node)
	{
		if (node == nullptr) {
			return;
		}
		identifier_table_.insert({node->identifier(), &(*node)});
		program_.add_child(std::move(node));
	}

	void on_gate_definition(std::unique_ptr<decl_gate> node)
	{
		if (node == nullptr) {
			return;
		}
		identifier_table_.insert({node->identifier(), &(*node)});
		program_.add_child(std::move(node));
	}

	// FIXME: I wonder is this is the class which should mantain this
	// declaration table.
	void on_parameter_declaration(const decl_parameter* decl_parameter)
	{
		if (decl_parameter == nullptr) {
			return;
		}
		std::cout << "adding parameter: "
		          << decl_parameter->identifier() << '\n';
		scope_.insert({decl_parameter->identifier(), decl_parameter});
	}

	void on_argument_declaration(const decl_argument* decl_argument)
	{
		if (decl_argument == nullptr) {
			return;
		}
		std::cout << "adding argument: " << decl_argument->identifier()
		          << "|\n";
		scope_.insert({decl_argument->identifier(), decl_argument});
	}

	void on_gate_statement(std::unique_ptr<stmt_gate> stmt_gate)
	{
		if (stmt_gate == nullptr) {
			return;
		}
		program_.add_child(std::move(stmt_gate));
	}

	void on_qubit(std::string_view qubit_id)
	{
		qubits_.insert(qubit_id);
	}

	std::unique_ptr<program> finish()
	{
		for (auto id : qubits_) {
			program_.add_qubit(id);
		}
		return program_.finish();
	}

private:
	const ast_node* find_declaration(std::string_view identifier) const
	{
		std::cout << "local scope: " << scope_.size() << '\n';
		std::cout << "find: " << identifier << '\n';
		auto param = scope_.find(identifier);
		if (param != scope_.end()) {
			return param->second;
		}
		std::cout << "global scope: " << identifier_table_.size() << '\n';
		auto entry = identifier_table_.find(identifier);
		if (entry != identifier_table_.end()) {
			return entry->second;
		}
		std::cout << "not found\n";
		return nullptr;
	}
};

} // namespace quil
} // namespace tweedledee
