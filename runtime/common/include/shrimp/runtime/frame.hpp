#ifndef SHRIMP_RUNTIME_FRAME_HPP
#define SHRIMP_RUNTIME_FRAME_HPP

#include <memory>
#include <vector>

#include <shrimp/runtime/register.hpp>
#include <shrimp/common/types.hpp>

namespace shrimp::runtime {

class Frame {
public:
    explicit Frame(std::shared_ptr<RuntimeFunc> func) : curr_method_(func), regs_(curr_method_->num_of_vregs) {}
    void setRetPc(const Byte *return_pc) noexcept
    {
        return_pc_ = return_pc;
    }
    const Byte *getRetPc() const noexcept
    {
        return return_pc_;
    }
    Register getReg(size_t idx) const noexcept
    {
        return regs_[idx];
    }
    void setReg(uint64_t val, uint8_t reg_num)
    {
        regs_[reg_num].setValue(val);
    }
    ByteOffset getOffsetToFunc() const noexcept
    {
        return curr_method_->func_start;
    }

private:
    std::shared_ptr<RuntimeFunc> curr_method_;
    std::vector<Register> regs_ {};
    const Byte *return_pc_ = nullptr;
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_FRAME_HPP
