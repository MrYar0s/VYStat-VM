#include <iostream>

#include <shrimp/runtime/shrimp_vm/interpreter.hpp>
#include <shrimp/runtime/shrimp_vm/interpreter-inl.hpp>

namespace shrimp::interpreter {

int runImpl(const InstType *pc, Frame *frame)
{
    dispatch_table[getOpcode(pc)](pc, frame);
    return 0;
}

}  // namespace shrimp::interpreter