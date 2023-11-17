#ifndef SHRIMP_COMMON_TYPES_HPP
#define SHRIMP_COMMON_TYPES_HPP

#include <cstdint>

using std::size_t;

namespace shrimp {

using Byte = uint8_t;
using HWord = uint16_t;
using Word = uint32_t;

// Common instruction size
using DWord = uint64_t;

// Byte offset in bin code
using ByteOffset = int64_t;

// Register index in range [0, 255]
using R8Id = uint8_t;

enum class ValueTag : uint8_t { INT };

enum class IntrinsicCode : uint8_t { PRINT_I32, PRINT_F, SCAN_I32, SCAN_F, SIN, COS, SQRT };

}  // namespace shrimp

#endif  // SHRIMP_COMMON_TYPES_HPP
