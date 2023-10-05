#ifndef RUNTIME_RUNTIME_HPP
#define RUNTIME_RUNTIME_HPP

#include <cstddef>
#include <array>
#include <cstdint>
#include "register.hpp"
#include "accumulator.hpp"
#include "types.hpp"

namespace shrimp {

class ShrimpVM;
class Frame;

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

#endif  // RUNTIME_RUNTIME_HPP