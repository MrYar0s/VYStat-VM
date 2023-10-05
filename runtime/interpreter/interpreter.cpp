#include "interpreter.hpp"
#include "interpreter-inl.hpp"
#include <iostream>

namespace shrimp::interpreter {

int runImpl(const InstType *pc, Frame *frame)
{
    dispatch_table[getOpcode(pc)](pc, frame);
    return 0;
}

}  // namespace shrimp::interpreter