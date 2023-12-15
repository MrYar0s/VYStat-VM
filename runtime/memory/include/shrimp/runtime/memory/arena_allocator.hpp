#ifndef RUNTIME_MEMORY_ARENA_ALLOCATOR_HPP
#define RUNTIME_MEMORY_ARENA_ALLOCATOR_HPP

#include <memory_resource>

#include <shrimp/common/types.hpp>

namespace shrimp::runtime {

// Effectively allocates raw memory blocks
using ArenaAllocator = std::pmr::monotonic_buffer_resource;

// Allocator to be used by containers. Requires ArenaAllocator to construct
template<class T>
using ContainerAllocator = std::pmr::polymorphic_allocator<T>;

}  // namespace shrimp::runtime

#endif  // RUNTIME_MEMORY_ARENA_ALLOCATOR_HPP
