#ifndef COMMON_BITOPS_HPP
#define COMMON_BITOPS_HPP

#include <cstdint>

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
    return *reinterpret_cast<uint64_t *>(&val);
}

template <typename Type>
Type getValue(uint64_t val)
{
    return *reinterpret_cast<Type *>(&val);
}

#endif // COMMON_BITOPS_HPP