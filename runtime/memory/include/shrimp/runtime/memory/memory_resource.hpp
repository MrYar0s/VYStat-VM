#ifndef RUNTIME_MEMORY_MEMORY_RESOURCE_HPP
#define RUNTIME_MEMORY_MEMORY_RESOURCE_HPP

#include <list>
#include <memory_resource>

#include <shrimp/common/types.hpp>

namespace shrimp::runtime {

// Allocates raw memory blocks until limit is reached
class LimitedArena final : public std::pmr::memory_resource {
    void *begin_ = nullptr;
    void *curr_pos_ = nullptr;
    size_t space_ = 0;

public:
    LimitedArena(size_t limit) : begin_(new uint8_t[limit]), curr_pos_(begin_), space_(limit)
    {
        assert(limit != 0);
        assert(begin_ != nullptr);
    }

    ~LimitedArena()
    {
        delete[] static_cast<uint8_t *>(begin_);
    }

    LimitedArena(const LimitedArena &) = delete;
    LimitedArena(LimitedArena &&) = delete;

    LimitedArena &operator=(const LimitedArena &) = delete;
    LimitedArena &operator=(LimitedArena &&) = delete;

    void *do_allocate(size_t bytes, size_t alignment) override
    {
        void *aligned_pos = std::align(alignment, bytes, curr_pos_, space_);

        if (aligned_pos == nullptr) {
            return nullptr;
        }

        curr_pos_ = static_cast<uint8_t *>(curr_pos_) + bytes;
        space_ -= bytes;

        return aligned_pos;
    }

    void do_deallocate(void * /*p*/, size_t /*bytes*/, size_t /*alignment*/) override
    {
        /* do nothing */
    }

    bool do_is_equal(const memory_resource &other) const noexcept override
    {
        return this == &other;
    }
};

struct AllocationInfo final {
    void *ptr = nullptr;
    size_t size = 0;
    bool isFree = true;
};

class LimitedMemRes final : public std::pmr::memory_resource {
    size_t allocated_ = 0;
    std::list<AllocationInfo> allocations_;
    std::pmr::unsynchronized_pool_resource allocator_ {};

    void *do_allocate(size_t bytes, size_t alignment) override
    {
        void *out = allocator_.allocate(bytes, alignment);

        if (out != nullptr) {
            allocations_.push_back(AllocationInfo{out, bytes, false});
            allocated_ += bytes;
        }

        return out;
    }

    void do_deallocate(void *p, size_t bytes, size_t alignment) override
    {
        allocator_.deallocate(p, bytes, alignment);
        allocated_ -= bytes;
        // allocations_ are changed by GC
    }

public:
    LimitedMemRes(LimitedArena &arena) : allocator_(&arena) {}

    auto &getAllocated() noexcept
    {
        return allocated_;
    }

    auto &getAllocations() noexcept
    {
        return allocations_;
    }

    bool do_is_equal(const memory_resource &other) const noexcept override
    {
        return this == &other;
    }
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_MEMORY_MEMORY_RESOURCE_HPP
