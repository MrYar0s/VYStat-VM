#ifndef RUNTIME_INTERPRETER_INTERPRETER_HPP
#define RUNTIME_INTERPRETER_INTERPRETER_HPP

#include <cstdint>
#include "frame.hpp"

namespace shrimp::interpreter {

int runImpl(const InstType *pc, Frame *frame);

}  // namespace shrimp::interpreter

#endif  // RUNTIME_INTERPRETER_INTERPRETER_HPP