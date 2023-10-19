#ifndef SHRIMP_RUNTIME_RUNTIME_HPP
#define SHRIMP_RUNTIME_RUNTIME_HPP

#include <cstddef>
#include <array>
#include <cstdint>

#include <shrimp/common/types.hpp>

#include <shrimp/runtime/register.hpp>
#include <shrimp/runtime/accumulator.hpp>
#include <shrimp/runtime/frame.hpp>

namespace shrimp {

class ShrimpVM;

class Runtime final {
public:
    Runtime(ShrimpVM *vm);
    Frame *getCurrFrame()
    {
        return curr_frame_;
    }
    int runImpl();

private:
    ShrimpVM *vm_;
    Frame *curr_frame_;
    Accumulator acc_ {0};
};

}  // namespace shrimp

#endif  // SHRIMP_RUNTIME_RUNTIME_HPP