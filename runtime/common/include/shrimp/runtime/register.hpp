#ifndef SHRIMP_RUNTIME_REGISTER_HPP
#define SHRIMP_RUNTIME_REGISTER_HPP

#include <cstdint>

namespace shrimp::runtime {

class Register final {
public:
    uint64_t getValue()
    {
        return val_;
    }
    void setValue(uint64_t val)
    {
        val_ = val;
    }

private:
    uint64_t val_ = 0;
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_REGISTER_HPP
