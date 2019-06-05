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

class list_ids
    : public ast_node
    , public ast_node_container<list_ids, ast_node> {
public:
	class builder {
	public:
		explicit builder(ast_context* ctx, uint32_t location)
		    : node_(new (*ctx) list_ids(location))
		{}

		void add_child(ast_node* child)
		{
			node_->add_child(child);
		}

		list_ids& get()
		{
			return *node_;
		}

		list_ids* finish()
		{
			return node_;
		}

	private:
		list_ids* node_;
	};

private:
	list_ids(uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::list_ids;
	}
};

} // namespace qasm
} // namespace tweedledee
