#ifndef SHRIMP_RUNTIME_ACCUMULATOR_HPP
#define SHRIMP_RUNTIME_ACCUMULATOR_HPP

#include <cstdint>

namespace shrimp::runtime {

class Accumulator final {
public:
    explicit Accumulator(uint64_t val) : val_ {val} {}
    uint64_t getValue()
    {
        return val_;
    }
    void setValue(uint64_t val)
    {
        val_ = val;
    }

private:
    uint64_t val_;
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_ACCUMULATOR_HPP
