/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "ast.hpp"

namespace tweedledee {
namespace quil {

class ast_node;

enum class visitor_info: unsigned short {
	container_begin,
	container_end,
	leaf,
};

namespace detail {

using visitor_callback_t = bool (*)(void* mem, const ast_node&, visitor_info info);
inline bool visit(const ast_node& node, visitor_callback_t cb, void* functor);

template <typename Func>
bool visitor_callback(void* mem, const ast_node& node, visitor_info info)
{
	auto& func = *static_cast<Func*>(mem);
	return func(node, info);
}

template <typename T>
bool handle_container(const ast_node& node, detail::visitor_callback_t cb, void* functor)
{
	auto& container = static_cast<const T&>(node);
	auto handle_children = cb(functor, container, visitor_info::container_begin);
	if (handle_children) {
		for (auto& child : container)
			if (!detail::visit(child, cb, functor))
				return false;
	}
	return cb(functor, container, visitor_info::container_end);
}

bool visit(const ast_node& node, visitor_callback_t cb, void* functor)
{
	switch (node.kind()) {
	case ast_node_kinds::expr_binary_op:
		return handle_container<expr_binary_op>(node, cb, functor);

	case ast_node_kinds::stmt_gate:
		return handle_container<stmt_gate>(node, cb, functor);

	case ast_node_kinds::program:
		return handle_container<program>(node, cb, functor);

	case ast_node_kinds::expr_sign:
		return handle_container<expr_sign>(node, cb, functor);

	case ast_node_kinds::qubit:
	case ast_node_kinds::expr_integer:
	case ast_node_kinds::expr_real:
		return cb(functor, node, visitor_info::leaf);

	default:
		break;
	}
	return true;
}

} // namespace detail

template <typename Func>
void visit(const ast_node& node, Func f)
{ detail::visit(node, &detail::visitor_callback<Func>, &f); }

} // namespace quil
} // namespace tweedledee
