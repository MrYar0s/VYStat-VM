#ifndef SHRIMP_RUNTIME_SHRIMP_VM_HPP
#define SHRIMP_RUNTIME_SHRIMP_VM_HPP

#include <vector>
#include <cstdint>
#include <stack>
#include <cassert>

#include <shrimp/runtime/frame.hpp>
#include <shrimp/runtime/runtime.hpp>

namespace shrimp::runtime {

class ShrimpVM final {
public:
    ShrimpVM(std::vector<Byte> code) : code_(std::move(code))
    {
        stack_.push(Frame {});
    }

    int runImpl();

    const Byte *&pc() noexcept
    {
        return pc_;
    }

    const Byte *getEntryPoint() const noexcept
    {
        return code_.data();
    }

    void setRuntime(Runtime *runtime)
    {
        runtime_ = runtime;
    }

    auto &acc() noexcept
    {
        return acc_;
    }

    auto &stack() noexcept
    {
        return stack_;
    }

    auto &currFrame() noexcept
    {
        return stack_.top();
    }

private:
    Runtime *runtime_ = nullptr;

    std::vector<Byte> code_ {};
    const Byte *pc_ = code_.data();

    Register acc_ {};
    std::stack<Frame> stack_ {};
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_SHRIMP_VM_HPP
