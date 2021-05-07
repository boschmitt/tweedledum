/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

namespace tweedledum {

/*! \brief The base class documents the required public interface exposed by all
 * allocators
 *
 * Using CRTP idiom this class also redirects all of the overloads to a single
 * core set of methods which the derived class must define.
 */
template<typename DerivedT>
class AllocatorBase {
public:
    /*! \brief Allocates `size` bytes of `alignment` aligned memory */
    // This method must be implemented by DerivedT.
    void* allocate(size_t const size, size_t const alignment)
    {
        static_assert(static_cast<void* (AllocatorBase::*) (size_t, size_t)>(
                        &AllocatorBase::allocate)
                        != static_cast<void* (DerivedT::*) (size_t, size_t)>(
                          &DerivedT::allocate),
          "Class derives from AllocatorBase without implementing the core "
          "allocate(size_t, size_t) overload!");
        return static_cast<DerivedT*>(this)->allocate(size, alignment);
    }

    /*! \brief Dallocates `ptr` to `size` bytes of memory _allocated by this
     * allocator_ */
    // This method must be implemented by DerivedT.
    void deallocate(void const* ptr, size_t const size)
    {
        static_assert(static_cast<void (AllocatorBase::*)(void const*, size_t)>(
                        &AllocatorBase::deallocate)
                        != static_cast<void (DerivedT::*)(void const*, size_t)>(
                          &DerivedT::deallocate),
          "Class derives from AllocatorBase without implementing the "
          "core deallocate(void *) overload!");
        return static_cast<DerivedT*>(this)->deallocate(ptr, size);
    }

    /*! \brief Allocate space for a sequence objects without constructing them.
     *
     * \paramt T the type of the objects in the sequence
     * \param[in] num the number of objects in the sequence.
     * \returns a pointer to the allocated memory.
     */
    template<typename T>
    T* allocate(size_t const num = 1)
    {
        return static_cast<T*>(allocate(num * sizeof(T), alignof(T)));
    }

    /*! \brief Deallocate space for a sequence of `num` objects of type `T`. */
    template<typename T>
    std::enable_if_t<!std::is_same<std::remove_cv_t<T>, void>::value, void>
    deallocate(T* ptr, size_t const num = 1)
    {
        deallocate(static_cast<void const*>(ptr), num * sizeof(T));
    }
};

class MallocAllocator : public AllocatorBase<MallocAllocator> {
public:
    void* allocate(size_t const size, size_t /* alignment */)
    {
        return safe_malloc(size);
    }
    using AllocatorBase<MallocAllocator>::allocate;

    void deallocate(void const* ptr, size_t /* size */)
    {
        std::free(const_cast<void*>(ptr));
    }
    using AllocatorBase<MallocAllocator>::deallocate;

private:
    inline void* safe_malloc(size_t const size)
    {
        void* result = std::malloc(size);
        if (result == nullptr) {
            std::cerr << "std::malloc(): Allocation failed.\n";
            std::cerr << "tweedledum ERROR: out of memory (OOM)\n";
            std::abort();
        }
        return result;
    }

    inline void* safe_calloc(size_t const count, size_t const size)
    {
        void* result = std::calloc(count, size);
        if (result == nullptr) {
            std::cerr << "std::calloc(): Allocation failed.\n";
            std::cerr << "tweedledum ERROR: out of memory (OOM)\n";
            std::abort();
        }
        return result;
    }

    inline void* safe_realloc(void* ptr, size_t const size)
    {
        void* result = std::realloc(ptr, size);
        if (result == nullptr) {
            std::cerr << "std::realloc(): Allocation failed.\n";
            std::cerr << "tweedledum ERROR: out of memory (OOM)\n";
            std::abort();
        }
        return result;
    }
};

/*! \brief Bump allocator (a.k.a Bump-pointer allocator)
 *
 * The so called Bump allocation is a fast, but limited technique to allocation.
 * We start with a chunk of memory, and we maintain a pointer within that
 * memory.  Whenever we allocate an object, we quickly test that we have enough
 * capacity left in the chunk to allocate the object and then increment the
 * pointer by the object's size.  If we do not have enough space in the chunk we
 * simply allocate a new one.  Basically, the allocator is a monotonically
 * growing pool of memory
 *
 * The disadvantage of bump allocation is that there is no general way to
 * deallocate individual objects or reclaim the memory region for a
 * no-longer-in-use object.
 */
template<typename AllocatorT = MallocAllocator, size_t ChunkSize = 4096,
  size_t SizeThreshold = ChunkSize>
class BumpAllocatorImpl
    : public AllocatorBase<
        BumpAllocatorImpl<AllocatorT, ChunkSize, SizeThreshold>> {
public:
    static_assert(SizeThreshold <= ChunkSize,
      "The SizeThreshold must be at most the ChunkSize to ensure "
      "that objects larger than a chunk go into their own memory "
      "allocation.");

    BumpAllocatorImpl() = default;

    template<typename T>
    BumpAllocatorImpl(T&& allocator_)
        : allocator_(std::forward<T&&>(allocator_))
    {}

    BumpAllocatorImpl(BumpAllocatorImpl&& other)
        : current_ptr_(other.current_ptr_)
        , end_ptr_(other.end_ptr_)
        , chunks_(std::move(other.chunks_))
        , custom_sized_chunks_(std::move(other.custom_sized_chunks_))
        , num_bytes_allocated_(other.num_bytes_allocated_)
        , allocator_(std::move(other.allocator_))
    {
        other.current_ptr_ = nullptr;
        other.end_ptr_ = nullptr;
        other.num_bytes_allocated_ = 0;
        other.chunks_.clear();
        other.custom_sized_chunks_.clear();
    }

    ~BumpAllocatorImpl()
    {
        for (auto i = chunks_.begin(), end = chunks_.end(); i != end; ++i) {
            size_t size = compute_chunk_size(std::distance(chunks_.begin(), i));
            allocator_.deallocate(*i, size);
        }
        for (auto& [ptr, size] : custom_sized_chunks_) {
            allocator_.deallocate(ptr, size);
        }
    }

    BumpAllocatorImpl& operator=(BumpAllocatorImpl&& rhs)
    {
        current_ptr_ = rhs.current_ptr_;
        end_ptr_ = rhs.end_ptr_;
        num_bytes_allocated_ = rhs.num_bytes_allocated_;
        chunks_ = std::move(rhs.chunks_);
        custom_sized_chunks_ = std::move(rhs.custom_sized_chunks_);
        allocator_ = std::move(rhs.allocator_);

        rhs.current_ptr_ = rhs.end_ptr_ = nullptr;
        rhs.num_bytes_allocated_ = 0;
        rhs.chunks_.clear();
        rhs.custom_sized_chunks_.clear();
        return *this;
    }

    /*! \brief Allocate `size` bytes at the specified `alignment`. */
    void* allocate(size_t const size, size_t const alignment)
    {
        assert(
          alignment > 0 && "0-byte alignnment is not allowed. Use 1 instead.");

        // Keep track of how many bytes we've allocated.
        num_bytes_allocated_ += size;

        size_t adjustment = alignment_adjustment(current_ptr_, alignment);
        assert(
          adjustment + size >= size && "adjustment + size must not overflow");

        // Check if we have enough space.
        if (adjustment + size <= size_t(end_ptr_ - current_ptr_)) {
            char* aligned_ptr = current_ptr_ + adjustment;
            current_ptr_ = aligned_ptr + size;
            return aligned_ptr;
        }

        // If Size is really big, allocate a separate chunk for it.
        size_t padded_size = size + alignment - 1;
        if (padded_size > SizeThreshold) {
            void* new_chunk = allocator_.allocate(padded_size, 0);
            custom_sized_chunks_.push_back(
              std::make_pair(new_chunk, padded_size));

            uintptr_t aligned_address = align_address(new_chunk, alignment);
            assert(
              aligned_address + size <= (uintptr_t) new_chunk + padded_size);
            char* aligned_ptr = (char*) aligned_address;
            return aligned_ptr;
        }

        // Otherwise, start a new chunk and try again.
        new_chunk();
        uintptr_t aligned_address = align_address(current_ptr_, alignment);
        assert(aligned_address + size <= (uintptr_t) end_ptr_
               && "Unable to allocate memory!");
        char* aligned_ptr = (char*) aligned_address;
        current_ptr_ = aligned_ptr + size;
        return aligned_ptr;
    }
    using AllocatorBase<BumpAllocatorImpl>::allocate;

    // This Bump allocator nevers free its storage;
    void deallocate(void const*, size_t)
    {}
    using AllocatorBase<BumpAllocatorImpl>::deallocate;

    /*! \brief Returns the number of allocated chunks. */
    size_t num_chunks() const
    {
        return chunks_.size() + custom_sized_chunks_.size();
    }

    /*! \brief Returns total memory usage in number of bytes. (including
     * alignment) */
    size_t total_memory() const
    {
        size_t total_memory = 0;
        for (auto i = chunks_.begin(), end = chunks_.end(); i != end; ++i) {
            total_memory +=
              compute_chunk_size(std::distance(chunks_.begin(), i));
        }
        for (auto& [_, size] : custom_sized_chunks_) {
            total_memory += size;
        }
        return total_memory;
    }

    /*! \brief Returns total number of resquested bytes. */
    // Used to calculate the number of wasted memory used with alignment,
    // padding etc..
    size_t num_bytes_allocated() const
    {
        return num_bytes_allocated_;
    }

    /*! \brief Print stats about the allocator in `std::cout`. */
    void print_stats() const
    {
        std::cout << "\nNumber of memory regions: " << num_chunks() << '\n'
                  << "Bytes used: " << num_bytes_allocated() << '\n'
                  << "Bytes allocated: " << total_memory() << '\n'
                  << "Bytes wasted: "
                  << (total_memory() - num_bytes_allocated())
                  << " (includes alignment, etc)\n";
    }

private:
#pragma region Helper functions
    /*! \brief Aligns `address` to `alignment` bytes, rounding up when
     * necessary. */
    static uintptr_t align_address(void const* address, size_t alignment)
    {
        assert(alignment && !(alignment & (alignment - 1))
               && "alignment is not a power of two!");
        assert((uintptr_t) address + alignment - 1 >= (uintptr_t) address);
        return (
          ((uintptr_t) address + alignment - 1) & ~(uintptr_t) (alignment - 1));
    }

    /*! \brief Returns the necessary adjustment for aligning `ptr` to
     * `alignment` */
    static size_t alignment_adjustment(void const* ptr, size_t alignment)
    {
        return align_address(ptr, alignment) - (uintptr_t) ptr;
    }

    static size_t compute_chunk_size(uint32_t chunk_idx)
    {
        // Scale the actual allocated chunk size based on the number of chunks
        // allocated. Every 128 chunks allocated, we double the allocated size
        // to reduce allocation frequency, but saturate at multiplying the chunk
        // size by 2^30.
        return ChunkSize
             * ((size_t) 1 << std::min<size_t>(30, chunk_idx / 128));
    }

    /*! \brief Allocate a new chunk */
    void new_chunk()
    {
        size_t size = compute_chunk_size(chunks_.size());
        void* new_chunk = allocator_.allocate(size, 0);
        chunks_.push_back(new_chunk);
        current_ptr_ = (char*) (new_chunk);
        end_ptr_ = ((char*) new_chunk) + size;
    }
#pragma endregion

    /*! \brief The current pointer into the current chunk (points to the next
     * free byte). */
    char* current_ptr_ = nullptr;

    /*! \brief The pointer to the end of the current chunk. */
    char* end_ptr_ = nullptr;

    /*! \brief The chunks allocated so far. */
    std::vector<void*> chunks_;

    /*! \brief Custom-sized chunks allocated for too-large allocation requests.
     */
    std::vector<std::pair<void*, size_t>> custom_sized_chunks_;

    /*! \brief Number of bytes allocated (requested). */
    size_t num_bytes_allocated_ = 0;

    // The allocator_ instance we use to get chunks of memory.
    AllocatorT allocator_;
};

// The standard `BumpAllocator` which just uses the default template parameters.
typedef BumpAllocatorImpl<> BumpAllocator;

} // namespace tweedledum