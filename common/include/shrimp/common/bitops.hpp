#ifndef SHRIMP_COMMON_BITOPS_HPP
#define SHRIMP_COMMON_BITOPS_HPP

#include <cstdint>
#include <bit>

using std::size_t;

namespace shrimp::bit {

template <uint64_t MASK>
uint64_t applyMask(uint64_t input)
{
    return input & MASK;
}

template <uint8_t SHIFT>
uint64_t applyLeftShift(uint64_t input)
{
    static_assert(SHIFT < sizeof(uint64_t) * 8);
    return input << SHIFT;
}

template <uint8_t SHIFT>
uint64_t applyRightShift(uint64_t input)
{
    static_assert(SHIFT < sizeof(uint64_t) * 8);
    return input >> SHIFT;
}

template <typename Type>
uint64_t castToWritable(Type val)
{
    if constexpr (sizeof(uint64_t) == sizeof(Type)) {
        return std::bit_cast<uint64_t>(val);
    } else {
        uint32_t res = std::bit_cast<uint32_t>(val);
        return static_cast<uint64_t>(res);
    }
}

template <typename Type>
Type getValue(uint64_t val)
{
    if constexpr (sizeof(uint64_t) == sizeof(Type)) {
        return std::bit_cast<Type>(val);
    } else {
        uint32_t res = static_cast<uint32_t>(val & 0xffffffff);
        return std::bit_cast<Type>(res);
    }
}

using BitSize = size_t;
using BitIdx = size_t;

static constexpr BitSize BYTE_BIT_SIZE = 8;

template <class T>
constexpr BitSize bitSize() noexcept
{
    return sizeof(T) * BYTE_BIT_SIZE;
}

template <typename UInt, BitIdx sign_idx>
constexpr UInt signExtend(UInt value) noexcept
{
    static_assert(std::is_unsigned_v<UInt>);
    static_assert(sign_idx < bitSize<UInt>());

    // Zero upper bits
    value &= (UInt {1} << (sign_idx + 1)) - 1;

    UInt sign_mask = UInt {1} << sign_idx;
    return (value ^ sign_mask) - sign_mask;
}

}  // namespace shrimp::bit

#endif  // SHRIMP_COMMON_BITOPS_HPP
