/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_context.hpp"
#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>

namespace tweedledee {
namespace qasm {

class list_exps
    : public ast_node
    , public ast_node_container<list_exps, ast_node> {
public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location)
		    : node_(new (*ctx) list_exps(location))
		{}

		void add_child(ast_node* child)
		{
			node_->add_child(child);
		}

		list_exps& get()
		{
			return *node_;
		}

		list_exps* finish()
		{
			return node_;
		}

	private:
		list_exps* node_;
	};

private:
	list_exps(uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::list_exps;
	}
};

} // namespace qasm
} // namespace tweedledee
