#ifndef RUNTIME_REGISTER_HPP
#define RUNTIME_REGISTER_HPP

#include <cstdint>

namespace shrimp {

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
    uint64_t val_;
};

}  // namespace shrimp

#endif  // RUNTIME_REGISTER_HPP