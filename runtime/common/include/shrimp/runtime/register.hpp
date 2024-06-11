#ifndef SHRIMP_RUNTIME_REGISTER_HPP
#define SHRIMP_RUNTIME_REGISTER_HPP

#include <cstdint>

namespace shrimp::runtime {

class Register final {
public:
    uint64_t getValue() const
    {
        return val_;
    }
    void setValue(uint64_t val, bool refMark)
    {
        val_ = val;
        refMark_ = refMark;
    }
    bool getRefMark() const
    {
        return refMark_;
    }

private:
    uint64_t val_ = 0;
    bool refMark_ = false;
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_REGISTER_HPP
