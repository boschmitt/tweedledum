/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>

// Based this implementation on what I have seen in LLVM's SmallVector

namespace tweedledum {
namespace detail {

class SmallVectorBase {
public:
    uint32_t size() const
    {
        return size_;
    }

    uint32_t capacity() const
    {
        return capacity_;
    }

    bool empty() const
    {
        return !size_;
    }

    void size(uint32_t new_size)
    {
        assert(new_size <= capacity());
        size_ = new_size;
    }

protected:
    void* begin_;
    uint32_t size_;
    uint32_t capacity_;

    static constexpr uint32_t max_size()
    {
        return std::numeric_limits<uint32_t>::max();
    }

    SmallVectorBase(void* begin, uint32_t capacity)
        : begin_(begin), size_(0u), capacity_(capacity)
    {}

    void grow_pod(void* begin, uint32_t min_size, uint32_t type_size)
    {
        if (min_size > max_size() || capacity() == max_size()) {
            assert(0);
            return;
        }

        uint32_t new_capacity = 2u * capacity() + 1u;
        new_capacity = std::min(std::max(new_capacity, min_size), max_size());

        void* new_mem;
        if (begin_ == begin) {
            new_mem = std::malloc(new_capacity * type_size);
            assert(new_mem != nullptr);
            std::memcpy(new_mem, this->begin_, size() * type_size);
        } else {
            new_mem = std::realloc(this->begin_, new_capacity * type_size);
            assert(new_mem != nullptr);
        }
        this->begin_ = new_mem;
        this->capacity_ = new_capacity;
    }
};

template <class T>
struct SmallVectorAlignment {
    std::aligned_storage_t<sizeof(SmallVectorBase), alignof(SmallVectorBase)> base;
    std::aligned_storage_t<sizeof(T), alignof(T)> first_element;
};

template <typename T>
class SmallVectorTBase : public SmallVectorBase {
public:
    using value_type = T;
    using size_type = uint32_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;
    using const_pointer = T const*;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using SmallVectorBase::capacity;
    using SmallVectorBase::empty;
    using SmallVectorBase::size;

    iterator begin()
    { 
        return static_cast<iterator>(this->begin_);
    }

    const_iterator begin() const
    {
        return static_cast<const_iterator>(this->begin_);
    }

    iterator end() 
    {
        return begin() + size();
    }

    const_iterator end() const
    {
        return begin() + size();
    }

    reverse_iterator rbegin() 
    {
        return reverse_iterator(end()); 
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    size_type max_size() const
    {
        return std::min(this->max_size(), size_type(this->max_size() / sizeof(T)));
    }

    reference operator[](size_type idx)
    {
        assert(idx < size());
        return begin()[idx];
    }

    const_reference operator[](size_type idx) const
    {
        assert(idx < size());
        return begin()[idx];
    }

    reference back()
    {
        assert(!empty());
        return end()[-1];
    }

    const_reference back() const
    {
        assert(!empty());
        return end()[-1];
    }

protected:
    SmallVectorTBase(size_type size)
        : SmallVectorBase(first_element(), size)
    {}

    void grow_pod(size_type min_size, size_type type_size)
    {
        SmallVectorBase::grow_pod(first_element(), min_size, type_size);
    }

    bool is_inline() const
    {
        return this->begin_ == first_element();
    }

    void reset_inline()
    {
        this->begin_ = first_element();
        this->size_ = 0;
        this->capacity_ = 0;
    }

private:
    void* first_element() const
    {
        return const_cast<void*>(reinterpret_cast<void const*>(
            reinterpret_cast<char const*>(this) +
            offsetof(SmallVectorAlignment<T>, first_element)));
    }
};

template <typename T, bool = (std::is_trivially_copy_constructible<T>::value) &&
                             (std::is_trivially_move_constructible<T>::value) &&
                              std::is_trivially_destructible<T>::value>
class SmallVectorT : public SmallVectorTBase<T> {
public:
    void push_back(T const& element)
    {
        if (this->size() >= this->capacity()) {
            this->grow();
        }
        new ((void*)this->end()) T(element);
        this->size(this->size() + 1);
    }

    void push_back(T&& element)
    {
        if (this->size() >= this->capacity()) {
            this->grow();
        }
        new ((void*)this->end()) T(std::move(element));
        this->size(this->size() + 1);
    }

    void pop_back()
    {
        this->size(this->size() - 1);
        this->end()->~T();
    }

protected:
    SmallVectorT(uint32_t size)
        : SmallVectorTBase<T>(size)
    {}

    static void destroy(T* begin, T* end)
    {
        // Destroy elements in reverse order.  This takes care of possible
        // relations the elements might have among themselves.
        while (begin != end) {
            --end;
            end->~T();
        }
    }

    template <typename It1, typename It2>
    static void uninitialized_copy(It1 begin, It1 end, It2 dest)
    {
        std::uninitialized_copy(begin, end, dest);
    }

    template <typename It1, typename It2>
    static void uninitialized_move(It1 begin, It1 end, It2 dest)
    {
        std::uninitialized_copy(std::make_move_iterator(begin),
                                std::make_move_iterator(end), dest);
    }

    void grow(uint32_t min_size = 0)
    {
        if (min_size > this->max_size() || this->capacity() == this->max_size()) {
            assert(0);
        }

        uint32_t new_capacity = 2 * this->capacity() + 2;
        new_capacity = std::min(std::max(new_capacity, min_size), this->max_size());
        T* new_mem = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));

        this->uninitialized_move(this->begin(), this->end(), new_mem);

        destroy(this->begin(), this->end());

        if (!this->is_inline()) {
            std::free(this->begin());
        }
        this->begin_ = new_mem;
        this->capacity_ = new_capacity;
    }
};

template <typename T>
class SmallVectorT<T, /* IsPOD */ true> : public SmallVectorTBase<T> {
public:
    void push_back(T const& element)
    {
        if (this->size() >= this->capacity()) {
            this->grow();
        }
        std::memcpy(reinterpret_cast<void*>(this->end()), &element, sizeof(T));
        this->size(this->size() + 1);
    }

    void pop_back()
    {
        this->size(this->size() - 1);
    }

protected:
    SmallVectorT(uint32_t size)
        : SmallVectorTBase<T>(size)
    {}

    static void destroy(T*, T*)
    {}

    template <typename It1, typename It2>
    static void uninitialized_move(It1 begin, It1 end, It2 dest)
    {
        uninitialized_copy(begin, end, dest);
    }

    template <typename It1, typename It2>
    static void uninitialized_copy(It1 begin, It1 end, It2 dest)
    {
        std::uninitialized_copy(begin, end, dest);
    }

    template <typename T1, typename T2, std::enable_if_t<std::is_same<std::remove_const_t<T1>, T2>::value> * = nullptr>
    static void uninitialized_copy(T1* begin, T1* end, T2* dest)
    {
        if (begin != end) {
            std::memcpy(reinterpret_cast<void*>(dest), begin, (end - begin) * sizeof(T));
        }
    }

    void grow(uint32_t min_size = 0)
    {
        this->grow_pod(min_size, sizeof(T));
    }
};

template <typename T>
class SmallVector : public SmallVectorT<T> {
public:
    using size_type = typename SmallVectorT<T>::size_type;
    using iterator = typename SmallVectorT<T>::iterator;
    using const_iterator = typename SmallVectorT<T>::const_iterator;
    using reference = typename SmallVectorT<T>::reference;

    SmallVector(SmallVector const&) = delete;

    ~SmallVector()
    {
        // At this point, the subclass destructor has already destroyed all
        // elements in the vector.  We just need to make sure we free our 
        // possible heap allocated memory:
        if (!this->is_inline()) {
            std::free(this->begin());
        }
    }

    void clear()
    {
        this->destroy(this->begin(), this->end());
        this->size_ = 0;
    }

    void reserve(uint32_t num_elements)
    {
        if (this->capacity() < num_elements) {
            this->grow(num_elements);
        }
    }

    template <typename It, typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<It>::iterator_category, std::input_iterator_tag>::value>>
    void append(It begin, It end)
    {
        size_type num_elements = std::distance(begin, end);
        if (num_elements > this->capacity() - this->size()) {
            this->grow(this->size() + num_elements);
        }
        this->uninitialized_copy(begin, end, this->end());
        this->size(this->size() + num_elements);
    }

    template <typename... ArgTypes>
    reference emplace_back(ArgTypes&& ...args)
    {
        if (this->size() >= this->capacity()) {
            this->grow();
        }
        new ((void*)this->end()) T(std::forward<ArgTypes>(args)...);
        this->size(this->size() + 1);
        return this->back();
    }

    SmallVector& operator=(SmallVector const& rhs);

    SmallVector& operator=(SmallVector&& rhs);

    bool operator==(SmallVector const& rhs) const
    {
        if (this->size() != rhs.size()) {
            return false;
        }
        return std::equal(this->begin(), this->end(), rhs.begin());
    }

    bool operator!=(SmallVector const& rhs) const
    {
        return !(*this == rhs);
    }

protected:
    explicit SmallVector(uint32_t size)
        : SmallVectorT<T>(size)
    {}
};

template <typename T>
SmallVector<T>& SmallVector<T>::operator=(SmallVector<T> const& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    uint32_t rhs_size = rhs.size();
    uint32_t size = this->size();
    if (size >= rhs_size) {
        iterator new_end;
        if (rhs_size) {
            new_end = std::copy(rhs.begin(), rhs.begin() + rhs_size, this->begin());
        } else {
            new_end = this->begin();
        }
        this->destroy(new_end, this->end());
        this->size(rhs_size);
        return *this;
    }
    if (this->capacity() < rhs_size) {
        this->destroy(this->begin(), this->end());
        this->size(0);
        size = 0;
        this->grow(rhs_size);
    } else if (size) {
        std::copy(rhs.begin(), rhs.begin() + size, this->begin());
    }
    this->uninitialized_copy(rhs.begin() + size, rhs.end(), this->begin() + size);
    this->size(rhs_size);
    return *this;
}

template <typename T>
SmallVector<T>& SmallVector<T>::operator=(SmallVector<T>&& rhs)
{
    if (this == &rhs) {
        return *this;
    }
    if (!rhs.is_inline()) {
        this->destroy(this->begin(), this->end());
        if (!this->is_inline()) {
            std::free(this->begin());
        }
        this->begin_ = rhs.begin_;
        this->size_ = rhs.size_;
        this->capacity_ = rhs.capacity_;
        rhs.reset_inline();
        return *this;
    }
    uint32_t const rhs_size = rhs.size();
    uint32_t size = this->size();
    if (size >= rhs_size) {
        iterator new_end = this->begin();
        if (rhs_size) {
            new_end = std::move(rhs.begin(), rhs.end(), new_end);
        }
        this->destroy(new_end, this->end());
        this->size(rhs_size);
        rhs.clear();
        return *this;
    }
    if (this->capacity() < rhs_size) {
        this->destroy(this->begin(), this->end());
        this->size(0);
        size = 0;
        this->grow(rhs_size);
    } else if (size) {
        std::move(rhs.begin(), rhs.begin() + size, this->begin());
    }
    this->uninitialized_move(rhs.begin() + size, rhs.end(), this->begin() + size);
    this->size(rhs_size);
    rhs.clear();
    return *this;
}

template <typename T, uint32_t NumElements>
struct SmallVectorStorage {
    std::aligned_storage_t<sizeof(T) * NumElements, alignof(T)> inline_buffer_;
};

template <typename T>
struct alignas(alignof(T)) SmallVectorStorage<T, 0> {
};

} // namespace detail

template <typename T, uint32_t NumElements>
class SmallVector : public detail::SmallVector<T>, 
                           detail::SmallVectorStorage<T, NumElements> {
public:
    SmallVector()
        : detail::SmallVector<T>(NumElements) 
    {}

    ~SmallVector()
    {
        this->destroy(this->begin(), this->end());
    }

    template <typename It, typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<It>::iterator_category, std::input_iterator_tag>::value>>
    SmallVector(It begin, It end)
        : detail::SmallVector<T>(NumElements)
    {
        this->append(begin, end);
    }

    SmallVector(SmallVector const& other)
        : detail::SmallVector<T>(NumElements)
    {
        if (!other.empty()) {
            detail::SmallVector<T>::operator=(other);
        }
    }

    SmallVector(SmallVector&& other)
        : detail::SmallVector<T>(NumElements)
    {
        if (!other.empty()) {
            detail::SmallVector<T>::operator=(std::move(other));
        }
    }

    SmallVector const& operator=(SmallVector const& rhs)
    {
        detail::SmallVector<T>::operator=(rhs);
        return *this;
    }

    SmallVector const& operator=(SmallVector&& rhs)
    {
        detail::SmallVector<T>::operator=(std::move(rhs));
        return *this;
    }
};

} // namespace tweedledum
