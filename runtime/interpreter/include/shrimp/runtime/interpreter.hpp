#ifndef SHRIMP_RUNTIME_INTERPRETER_HPP
#define SHRIMP_RUNTIME_INTERPRETER_HPP

#include <cstdint>

#include <shrimp/common/types.hpp>
#include <shrimp/runtime/frame.hpp>

#include <shrimp/runtime/shrimp_vm.hpp>

namespace shrimp::runtime::interpreter {

int runImpl(ShrimpVM *vm);

}  // namespace shrimp::runtime::interpreter

#endif  // SHRIMP_RUNTIME_INTERPRETER_HPP
