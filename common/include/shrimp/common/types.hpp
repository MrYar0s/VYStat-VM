#ifndef COMMON_TYPES_HPP
#define COMMON_TYPES_HPP

#include <cstdint>

namespace shrimp {

using HWord = uint16_t;
using Word = uint32_t;

// Common instruction size
using DWord = uint64_t;

// Byte offset in bin code
using ByteOffset = int64_t;

// Register index in range [0, 255]
using R8Id = uint8_t;

// Deprecated. TODO: Replace with ByteOffset
// Offset in double words
using DWordOffset = int64_t;

// Deprecated. TODO: Replace with HWord, Word, DWord
using InstType = uint64_t;

enum class ValueTag : uint8_t { INT };

enum class IntrinsicCode : uint8_t { PRINT_I32, PRINT_F, SCAN_I32, SCAN_F, SIN, COS, SQRT };

}  // namespace shrimp

#endif  // COMMON_TYPES_HPP