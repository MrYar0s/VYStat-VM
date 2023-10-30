#include <shrimp/runtime/shrimp_vm.hpp>
#include <shrimp/runtime/interpreter.hpp>

namespace shrimp::runtime {

int ShrimpVM::runImpl()
{
    assert(!stack_.empty());
    interpreter::runImpl(this);
    return 0;
}

}  // namespace shrimp::runtime
