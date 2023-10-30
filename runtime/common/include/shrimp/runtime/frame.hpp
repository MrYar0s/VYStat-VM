#ifndef SHRIMP_RUNTIME_FRAME_HPP
#define SHRIMP_RUNTIME_FRAME_HPP

#include <vector>

#include <shrimp/runtime/register.hpp>

namespace shrimp::runtime {

class Frame {
public:
    Register getReg(size_t idx)
    {
        return regs_[idx];
    }
    void setReg(uint64_t val, uint8_t reg_num)
    {
        regs_[reg_num].setValue(val);
    }

private:
    /*  Doesn't need now */
    //  Method *curr_method_;
    std::vector<Register> regs_ {16};
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_FRAME_HPP
