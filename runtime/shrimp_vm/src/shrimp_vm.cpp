#include <shrimp/runtime/shrimp_vm.hpp>
#include <shrimp/runtime/interpreter.hpp>
#include <shrimp/runtime/memory/gc.hpp>
#include "shrimp/common/logger.hpp"

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
    if (10 * getAllocator().getAllocated() < 9 * MEM_LIMIT) {
        return;
    }
    LOG_DEBUG("GC WAS TRIGGERED", getLogLevel());
    mem::GC gc {this};
    gc.run();
}

}  // namespace shrimp::runtime
