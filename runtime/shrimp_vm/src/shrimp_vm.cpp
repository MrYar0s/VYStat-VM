#include <shrimp/runtime/shrimp_vm.hpp>
#include <shrimp/runtime/interpreter.hpp>
#include <shrimp/runtime/memory/gc.hpp>

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

void ShrimpVM::triggerGCIfNeed()
{
    mem::GC gc {this};
    gc.run();
}

}  // namespace shrimp::runtime
