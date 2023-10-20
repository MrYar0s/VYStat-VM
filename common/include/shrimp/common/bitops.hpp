#ifndef SHRIMP_COMMON_BITOPS_HPP
#define SHRIMP_COMMON_BITOPS_HPP

#include <cstdint>
#include <bit>

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

}  // namespace shrimp::bit

#endif  // SHRIMP_COMMON_BITOPS_HPP
