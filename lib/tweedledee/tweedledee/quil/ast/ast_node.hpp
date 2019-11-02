/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "ast_node_kinds.hpp"
#include "detail/intrusive_list.hpp"

#include <array>
#include <memory>

namespace tweedledee {
namespace quil {

// Base class for all Quil AST nodes
class ast_node : detail::intrusive_list_node<ast_node> {
public:
	ast_node(const ast_node&) = delete;
	ast_node& operator=(const ast_node&) = delete;

	virtual ~ast_node() = default;

	ast_node_kinds kind() const
	{
		return do_get_kind();
	}

	ast_node const& parent() const
	{
		return *parent_;
	}

	uint32_t location() const
	{
		return location_;
	}

protected:
	ast_node(uint32_t location)
	    : location_(location)
	{}

	uint32_t config_bits_ = 0;

private:
	virtual ast_node_kinds do_get_kind() const = 0;

	void on_insert(const ast_node* parent)
	{
		parent_ = parent;
	}

private:
	uint32_t location_;
	ast_node const* parent_;

	template<typename T>
	friend struct detail::intrusive_list_access;

	friend detail::intrusive_list_node<ast_node>;
};

// Helper class for nodes that are containers. i.e not leafs
template<typename Derived, typename T>
class ast_node_container {
public:
	using iterator = typename detail::intrusive_list<T>::iterator;
	using const_iterator = typename detail::intrusive_list<T>::const_iterator;

	iterator begin()
	{
		return children_.begin();
	}

	iterator end()
	{
		return children_.end();
	}

	const_iterator begin() const
	{
		return children_.begin();
	}

	const_iterator end() const
	{
		return children_.end();
	}

	size_t num_children() const
	{
		return children_.size();
	}

protected:
	void add_child(T* ptr)
	{
		children_.push_back(static_cast<Derived*>(this), ptr);
	}

	~ast_node_container() = default;

private:
	detail::intrusive_list<T> children_;
};

} // namespace quil
} // namespace tweedledee
