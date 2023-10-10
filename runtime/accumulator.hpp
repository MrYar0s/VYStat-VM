#ifndef RUNTIME_ACCUMULATOR_HPP
#define RUNTIME_ACCUMULATOR_HPP

#include <cstdint>

namespace shrimp {

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

}  // namespace shrimp

#endif  // RUNTIME_ACCUMULATOR_HPP
