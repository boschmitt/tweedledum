/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <unordered_map>
#include <string>

namespace tweedledee {
namespace quil {

// Ast node kinds.
enum class ast_node_kinds : unsigned short {
	#define ASTNODE(X) X,
	#include "ast_node_kinds.def"
};

static const char * const ast_node_names[] = {
	#define ASTNODE(X) #X,
	#include "ast_node_kinds.def"
	nullptr
};

// Determines the name of a ast node as used within the front end.
// The name of a ast node will be an internal name.
inline const char *ast_node_name(ast_node_kinds k)
{
	auto k_idx = static_cast<int>(k);
	return ast_node_names[k_idx];
}

} // namespace quil
} // namespace tweedledee
