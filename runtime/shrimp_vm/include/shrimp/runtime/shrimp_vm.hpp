#ifndef SHRIMP_RUNTIME_SHRIMP_VM_HPP
#define SHRIMP_RUNTIME_SHRIMP_VM_HPP

#include <vector>
#include <cstdint>
#include <stack>

#include <shrimp/runtime/accumulator.hpp>
#include <shrimp/runtime/frame.hpp>
#include <shrimp/runtime/runtime.hpp>

namespace shrimp {

class ShrimpVM final {
public:
    ShrimpVM(std::vector<InstType> code) : code_(code), pc_ {code_.data()} {}
    InstType *getPc()
    {
        return pc_;
    }
    InstType *getEntrypoint()
    {
        return code_.data();
    }
    void setRuntime(Runtime *runtime)
    {
        runtime_ = runtime;
    }
    Frame *createFrame(Accumulator *acc)
    {
        Frame *frame = new Frame {acc};
        stk_frames_.push(frame);
        return stk_frames_.top();
    }
    void deleteFrame()
    {
        Frame *frame = stk_frames_.top();
        stk_frames_.pop();
        delete frame;
    }

private:
    std::vector<InstType> code_ {};
    InstType *pc_;
    Runtime *runtime_ = nullptr;
    std::stack<Frame *> stk_frames_;
};

}  // namespace shrimp

#endif  // SHRIMP_RUNTIME_SHRIMP_VM_HPP
