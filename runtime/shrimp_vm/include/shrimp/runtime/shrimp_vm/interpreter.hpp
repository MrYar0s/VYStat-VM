#ifndef RUNTIME_SHRIMP_VM_INTERPRETER_HPP
#define RUNTIME_SHRIMP_VM_INTERPRETER_HPP

#include <cstdint>

#include <shrimp/common/types.hpp>
#include <shrimp/runtime/frame.hpp>

namespace shrimp::runtime::interpreter {

int runImpl(const InstType *pc, Frame *frame);

}  // namespace shrimp::runtime::interpreter

#endif  // RUNTIME_SHRIMP_VM_INTERPRETER_HPP
