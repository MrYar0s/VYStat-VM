#ifndef SHRIMP_RUNTIME_SHRIMP_VM_HPP
#define SHRIMP_RUNTIME_SHRIMP_VM_HPP

#include <list>
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

#include <shrimp/runtime/memory/memory_resource.hpp>

namespace shrimp::runtime {

class ShrimpVM final {
public:
    ShrimpVM(std::vector<Byte> code, std::vector<shrimpfile::File::FileString> strings,
             std::vector<shrimpfile::File::FileFunction> funcs, std::vector<shrimpfile::File::FileClass> classes,
             LogLevel log_level)
        : log_level_(log_level), code_(std::move(code))
    {
        for (auto &&str : strings) {
            strings_.emplace(str.id, str.str);
        }
        for (auto &&func : funcs) {
            funcs_.emplace(func.id, RuntimeFunc {func.func_start, func.num_of_args, func.num_of_vregs, func.name});
        }
        for (auto &&klass : classes) {
            FieldAccessor fields;
            for (auto &&field : klass.fields) {
                fields.push_back(RuntimeField {field.is_ref == 1, field.size, field.offset, field.name});
            }
            classes_.push_back(RuntimeClass {{BaseClassType::DEFAULT}, klass.size, klass.name, std::move(fields)});
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
        stack_.push_back(Frame {std::make_shared<RuntimeFunc>(funcs_[entry_id])});
        stringClass_ = BaseClass {STRING};
        pc_ += stack_.back().getOffsetToFunc();
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

    inline auto &currFrame() noexcept
    {
        return stack_.back();
    }

    const std::string &resolveString(StrId str_id) noexcept
    {
        return strings_[str_id];
    }

    const RuntimeFunc &resolveFunc(FuncId func_id) noexcept
    {
        return funcs_[func_id];
    }

    const RuntimeField &resolveField(ClassId class_id, FieldId field_id) noexcept
    {
        return classes_[class_id].fields[field_id];
    }

    StrId addStringToAccessor(std::string str) noexcept
    {
        StrId str_id = strings_.size();
        strings_.emplace(str_id, std::move(str));
        return str_id;
    }
    const Byte *getPcFromStart(ByteOffset offset) noexcept
    {
        return code_.data() + offset;
    }

    auto &getAllocator() noexcept
    {
        return allocator_;
    }

    auto &getClasses() noexcept
    {
        return classes_;
    }

    auto &getArrays() noexcept
    {
        return arrays_;
    }

    auto &getStringClass() noexcept
    {
        return stringClass_;
    }

    void triggerGCIfNeed();

private:
    Runtime *runtime_ = nullptr;
    LogLevel log_level_ = LogLevel::NONE;

    std::vector<Byte> code_ {};
    const Byte *pc_ = code_.data();

    Register acc_ {};
    std::vector<Frame> stack_ {};

    StringAccessor strings_;
    FuncAccessor funcs_;
    ClassAccessor classes_;
    ArrayAccessor arrays_;

    BaseClass stringClass_;

    static constexpr size_t MEM_LIMIT = 0x2000000;  // 32Mb
    LimitedArena arena_ {MEM_LIMIT};
    LimitedMemRes allocator_ {arena_};
};

}  // namespace shrimp::runtime

#endif  // SHRIMP_RUNTIME_SHRIMP_VM_HPP
