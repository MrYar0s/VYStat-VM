#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include <cstdint>

namespace shrimp {

using InstType = uint64_t;

enum class ValueTag : uint8_t { INT };

}  // namespace shrimp

#endif  // COMMON_TYPES_HPP