#ifndef SHRIMP_RUNTIME_FRAME_HPP
#define SHRIMP_RUNTIME_FRAME_HPP

#include <vector>

#include <shrimp/runtime/register.hpp>
#include <shrimp/runtime/accumulator.hpp>

namespace shrimp {

class Frame {
public:
    explicit Frame(Accumulator *acc) : acc_(acc) {}
    Register getReg(size_t idx)
    {
        return regs_[idx];
    }
    Accumulator getAcc()
    {
        return *acc_;
    }
    Frame *getPrevFrame()
    {
        return prev_frame_;
    }
    void setReg(uint64_t val, uint8_t reg_num)
    {
        regs_[reg_num].setValue(val);
    }
    void setAcc(uint64_t val)
    {
        acc_->setValue(val);
    }

private:
    Frame *prev_frame_ = nullptr;
    /*  Doesn't need now */
    //  Method *curr_method_;
    Accumulator *acc_;
    std::vector<Register> regs_ {16};
};

}  // namespace shrimp

#endif  // SHRIMP_RUNTIME_FRAME_HPP
