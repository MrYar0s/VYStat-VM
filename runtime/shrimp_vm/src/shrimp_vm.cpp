#include <shrimp/runtime/shrimp_vm.hpp>
#include <shrimp/runtime/interpreter.hpp>

namespace shrimp::runtime {

int ShrimpVM::runImpl()
{
    assert(!stack_.empty());
    auto status = interpreter::runImpl(this);
    if (status != 0) {
        return -1;
    }
    return acc().getValue();
}

}  // namespace shrimp::runtime
