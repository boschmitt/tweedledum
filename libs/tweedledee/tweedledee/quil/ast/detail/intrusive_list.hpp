/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <iterator>
#include <memory>

namespace tweedledee {
namespace quil {

class program;

namespace detail {

// An intrusive list is one where the pointer to the next list node is stored
// in the same structure as the node data. This is normally A Bad Thing, as it
// ties the data to the specific list implementation
template<typename T>
class intrusive_list_node {
	std::unique_ptr<T> next_;

	void do_on_insert(const T* parent)
	{
		static_cast<T&>(*this).on_insert(parent);
	}

	template<typename U>
	friend struct intrusive_list_access;
};

template<typename T>
struct intrusive_list_access {
	template<typename U>
	static T* next(const U& obj)
	{
		static_assert(std::is_base_of<U, T>::value, "must be a base");
		return static_cast<T*>(obj.next_.get());
	}

	template<typename U>
	static T* next(U& obj, std::unique_ptr<T> node)
	{
		static_assert(std::is_base_of<U, T>::value, "must be a base");
		obj.next_ = std::move(node);
		return static_cast<T*>(obj.next_.get());
	}

	template<typename U, typename V>
	static void on_insert(U& obj, const V* parent)
	{
		obj.do_on_insert(parent);
	}
};

template<typename T>
class intrusive_list_iterator {
public:
	using value_type = T;
	using reference = T&;
	using pointer = T*;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::forward_iterator_tag;

	intrusive_list_iterator()
	    : current_(nullptr)
	{}

	reference operator*() const
	{
		return *current_;
	}

	pointer operator->() const
	{
		return current_;
	}

	intrusive_list_iterator& operator++()
	{
		current_ = intrusive_list_access<T>::next(*current_);
		return *this;
	}

	intrusive_list_iterator operator++(int)
	{
		auto tmp = *this;
		++(*this);
		return tmp;
	}

	friend bool operator==(const intrusive_list_iterator& rhs, const intrusive_list_iterator& lhs)
	{
		return rhs.current_ == lhs.current_;
	}

	friend bool operator!=(const intrusive_list_iterator& rhs, const intrusive_list_iterator& lhs)
	{
		return !(rhs == lhs);
	}

private:
	intrusive_list_iterator(T* ptr)
	    : current_(ptr)
	{}

	T* current_;

	template<typename U>
	friend class intrusive_list;
};

template<typename T>
class intrusive_list {
public:
	intrusive_list() = default;

	template<typename Dummy = T,
	         typename = typename std::enable_if<std::is_same<Dummy, program>::value>::type>
	void push_back(std::unique_ptr<T> obj)
	{
		push_back_impl(std::move(obj));
	}

	template<typename U, typename = typename std::enable_if<!std::is_same<T, program>::value, U>::type>
	void push_back(const U* parent, std::unique_ptr<T> obj)
	{
		push_back_impl(std::move(obj));
		intrusive_list_access<T>::on_insert(*last_, parent);
	}

	bool empty() const
	{
		return first_ == nullptr;
	}

	using iterator = intrusive_list_iterator<T>;
	using const_iterator = intrusive_list_iterator<const T>;

	iterator begin()
	{
		return iterator(first_.get());
	}

	iterator end()
	{
		return {};
	}

	iterator back()
	{
		return iterator(last_);
	}

	const_iterator begin() const
	{
		return const_iterator(first_.get());
	}

	const_iterator end() const
	{
		return {};
	}

	const_iterator back() const
	{
		return const_iterator(last_);
	}

private:
	void push_back_impl(std::unique_ptr<T> obj)
	{
		if (last_ != nullptr) {
			auto ptr = intrusive_list_access<T>::next(*last_, std::move(obj));
			last_ = ptr;
		} else {
			first_ = std::move(obj);
			last_ = first_.get();
		}
	}

	std::unique_ptr<T> first_;
	T* last_ = nullptr;
};

template<typename T>
class iteratable_intrusive_list {
public:
	iteratable_intrusive_list(const intrusive_list<T>& list)
	    : list_(list)
	{}

	bool empty() const
	{
		return list_.empty();
	}

	using iterator = typename intrusive_list<T>::const_iterator;

	iterator begin() const
	{
		return list_.begin();
	}

	iterator end() const
	{
		return list_.end();
	}

	iterator back() const
	{
		return list_.back();
	}

private:
	const intrusive_list<T>& list_;
};

} // namespace detail
} // namespace quil
} // namespace tweedledee
