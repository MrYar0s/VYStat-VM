#ifndef SHRIMP_RUNTIME_SHRIMP_VM_HPP
#define SHRIMP_RUNTIME_SHRIMP_VM_HPP

#include <vector>
#include <cstdint>
#include <stack>
#include <cassert>

#include <shrimp/common/logger.hpp>

#include <shrimp/runtime/frame.hpp>
#include <shrimp/runtime/runtime.hpp>

namespace shrimp::runtime {

class ShrimpVM final {
public:
    ShrimpVM(std::vector<Byte> code, LogLevel log_level) : log_level_(log_level), code_(std::move(code))
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

    [[nodiscard]] auto getLogLevel() const noexcept
    {
        return log_level_;
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
    LogLevel log_level_ = LogLevel::NONE;

    std::vector<Byte> code_ {};
    const Byte *pc_ = code_.data();

    Register acc_ {};
    std::stack<Frame> stack_ {};
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_SHRIMP_VM_HPP
