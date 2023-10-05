#include "runtime.hpp"
#include "shrimp_vm.hpp"
#include "interpreter.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>

namespace shrimp {

Runtime::Runtime(ShrimpVM *vm)
{
    vm_ = vm;
    curr_frame_ = vm_->createFrame(&acc_);
}

int Runtime::runImpl()
{
    assert(curr_frame_ != nullptr);
    interpreter::runImpl(vm_->getPc(), getCurrFrame());
    vm_->deleteFrame();
    return 0;
}

}  // namespace shrimp