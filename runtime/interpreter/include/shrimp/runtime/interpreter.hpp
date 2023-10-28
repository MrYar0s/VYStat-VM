#ifndef SHRIMP_RUNTIME_INTERPRETER_HPP
#define SHRIMP_RUNTIME_INTERPRETER_HPP

#include <cstdint>

#include <shrimp/common/types.hpp>
#include <shrimp/runtime/frame.hpp>

namespace shrimp::runtime::interpreter {

int runImpl(const Byte *pc, Frame *frame);

}  // namespace shrimp::runtime::interpreter

#endif  // SHRIMP_RUNTIME_INTERPRETER_HPP
