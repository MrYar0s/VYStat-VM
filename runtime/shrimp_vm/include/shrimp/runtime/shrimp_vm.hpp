#ifndef SHRIMP_RUNTIME_SHRIMP_VM_HPP
#define SHRIMP_RUNTIME_SHRIMP_VM_HPP

#include <vector>
#include <cstdint>
#include <stack>
#include <cassert>
#include <algorithm>

#include <shrimp/common/logger.hpp>

#include <shrimp/runtime/frame.hpp>
#include <shrimp/runtime/runtime.hpp>

#include <shrimp/shrimpfile.hpp>
#include <shrimp/common/types.hpp>

namespace shrimp::runtime {

class ShrimpVM final {
public:
    ShrimpVM(std::vector<Byte> code, std::vector<shrimpfile::File::FileString> strings,
             std::vector<shrimpfile::File::FileFunction> funcs, LogLevel log_level)
        : log_level_(log_level), code_(std::move(code))
    {
        for (auto &&str : strings) {
            strings_.emplace(str.id, str.str);
        }
        for (auto &&func : funcs) {
            funcs_.emplace(func.id, RuntimeFunc {func.func_start, func.num_of_args, func.num_of_vregs, func.name});
        }
        auto is_main = [](const std::pair<const unsigned int, shrimp::RuntimeFunc> &func_pair) {
            return func_pair.second.name == "main";
        };
        auto it = std::find_if(funcs_.begin(), funcs_.end(), is_main);
        if (it == std::end(funcs_)) {
            std::cerr << "Didn't find entrypoint" << std::endl;
            std::abort();
        }
        FuncId entry_id = it->first;
        stack_.emplace(std::make_shared<RuntimeFunc>(funcs_[entry_id]));
        pc_ += stack_.top().getOffsetToFunc();
    }

    int runImpl();

    const Byte *&pc() noexcept
    {
        return pc_;
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

    const std::string &resolveString(StrId str_id) noexcept
    {
        return strings_[str_id];
    }

    const RuntimeFunc &resolveFunc(FuncId func_id) noexcept
    {
        return funcs_[func_id];
    }

    StrId addStringToAccessor(std::string str) noexcept
    {
        StrId str_id = strings_.size();
        strings_.emplace(str_id, std::move(str));
        return str_id;
    }
    const Byte *getPcFromStart(ByteOffset offset)
    {
        return code_.data() + offset;
    }

private:
    Runtime *runtime_ = nullptr;
    LogLevel log_level_ = LogLevel::NONE;

    std::vector<Byte> code_ {};
    const Byte *pc_ = code_.data();

    Register acc_ {};
    std::stack<Frame> stack_ {};

    StringAccessor strings_;
    FuncAccessor funcs_;
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_SHRIMP_VM_HPP
