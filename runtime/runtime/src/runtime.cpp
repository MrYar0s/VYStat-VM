#include <cassert>
#include <cstddef>
#include <iostream>

#include <shrimp/runtime/runtime.hpp>
#include <shrimp/runtime/shrimp_vm.hpp>
#include <shrimp/runtime/shrimp_vm/interpreter.hpp>

namespace shrimp::runtime {

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

}  // namespace shrimp::runtime
