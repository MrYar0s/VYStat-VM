#ifndef SHRIMP_COMMON_TYPES_HPP
#define SHRIMP_COMMON_TYPES_HPP

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>

using std::size_t;

namespace shrimp {

using Byte = uint8_t;
using HWord = uint16_t;
using Word = uint32_t;

using StrId = uint16_t;

// Common instruction size
using DWord = uint64_t;

// Byte offset in bin code
using ByteOffset = int64_t;

// Register index in range [0, 255]
using R8Id = uint8_t;

// Function id
using FuncId = uint32_t;

// Claass id
using ClassId = uint32_t;

// Field id
using FieldId = uint32_t;

enum class IntrinsicCode : uint8_t { PRINT_I32, PRINT_F, PRINT_STR, CONCAT, SUBSTR, SCAN_I32, SCAN_F, SIN, COS, SQRT };

using StringAccessor = std::unordered_map<StrId, std::string>;

struct RuntimeFunc final {
    ByteOffset func_start = 0;
    uint8_t num_of_args = 0;
    uint16_t num_of_vregs = 0;
    std::string name = "";
};

using FuncAccessor = std::unordered_map<FuncId, RuntimeFunc>;

struct RuntimeField final {
    uint64_t size = 0;
    std::string name = "";
};

using FieldAccessor = std::unordered_map<FieldId, RuntimeField>;

struct RuntimeClass final {
    uint64_t size = 0;
    std::string name = "";
    FieldAccessor fields = {};
};

using ClassAccessor = std::unordered_map<ClassId, RuntimeClass>;

}  // namespace shrimp

#endif  // SHRIMP_COMMON_TYPES_HPP
