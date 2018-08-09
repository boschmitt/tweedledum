/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
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

public:
	semantic() = default;
	semantic(const semantic&) = delete;
	void operator=(const semantic&) = delete;

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
};

} // namespace quil
} // namespace tweedledee
