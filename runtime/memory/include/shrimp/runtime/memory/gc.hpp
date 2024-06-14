#ifndef RUNTIME_MEMORY_GC_HPP
#define RUNTIME_MEMORY_GC_HPP

#include <cstdint>
#include "shrimp/common/logger.hpp"
#include "shrimp/common/types.hpp"
#include "shrimp/runtime/coretypes/array.hpp"
#include "shrimp/runtime/coretypes/class.hpp"
#include "shrimp/runtime/memory/class_word.hpp"
#include "shrimp/runtime/memory/mark_word.hpp"
#include "shrimp/runtime/memory/memory_resource.hpp"
#include "shrimp/runtime/memory/object_header.hpp"
#include "shrimp/runtime/shrimp_vm.hpp"

namespace shrimp::runtime::mem {

class GC {
public:
    GC(ShrimpVM *vm) : vm_(vm)
    {
        stringClassWord_ = reinterpret_cast<ClassWord>(&vm_->getStringClass());
    }
    void run()
    {
        LOG_DEBUG("[BEFORE GC] Allocations: " << vm_->getAllocator().getAllocations().size(), vm_->getLogLevel());
        LOG_DEBUG("[BEFORE GC] Allocated : " << vm_->getAllocator().getAllocated(), vm_->getLogLevel());
        collectRoots();
        LOG_DEBUG("Start of marking", vm_->getLogLevel());
        mark(MarkWord::GCState::MARKED);
        LOG_DEBUG("End of marking", vm_->getLogLevel());
        sweep();
        LOG_DEBUG("Start of post-marking", vm_->getLogLevel());
        mark(MarkWord::GCState::UNMARKED);
        LOG_DEBUG("End of post-marking", vm_->getLogLevel());
        LOG_DEBUG("[AFTER GC] Allocated : " << vm_->getAllocator().getAllocated(), vm_->getLogLevel());
        LOG_DEBUG("[AFTER GC] Allocations: " << vm_->getAllocator().getAllocations().size(), vm_->getLogLevel());
    }

private:
    void collectRoots()
    {
        LOG_DEBUG("Start of collecting", vm_->getLogLevel());
        for (auto &stack : vm_->stack()) {
            for (size_t i = 0; i < 256; i++) {
                const auto &reg = stack.getReg(i);
                if (reg.getRefMark() == 1) {
                    LOG_DEBUG("Register : " << i << "; Value : " << reg.getValue(), vm_->getLogLevel());
                    roots_.push_back(reinterpret_cast<ObjectHeader *>(reg.getValue()));
                }
            }
        }
        if (vm_->acc().getRefMark() == 1) {
            LOG_DEBUG("Register : Acc; Value : " << vm_->acc().getValue(), vm_->getLogLevel());
            roots_.push_back(reinterpret_cast<ObjectHeader *>(vm_->acc().getValue()));
        }
        LOG_DEBUG("End of collecting", vm_->getLogLevel());
    }

    void markClass(Class *klass, MarkWord::GCState state)
    {
        LOG_DEBUG("Class " << klass, vm_->getLogLevel());
        if (klass->getGCState() == state) {
            return;
        }
        auto classWord = klass->getClassWord();
        auto runtimeClass = reinterpret_cast<RuntimeClass *>(classWord);

        for (const auto &field : runtimeClass->fields) {
            if (!field.is_ref) {
                continue;
            }
            auto refField = reinterpret_cast<Class *>(klass->getField(field));
            if (refField == 0) {
                continue;
            }
            markClass(refField, state);
        }
        klass->setGCState(state);
    }

    void markArray(Array *arr, MarkWord::GCState state)
    {
        LOG_DEBUG("Array " << arr, vm_->getLogLevel());
        if (arr->getGCState() == state) {
            return;
        }
        auto classWord = arr->getClassWord();
        arr->setGCState(state);

        auto array = reinterpret_cast<RuntimeArray *>(classWord);
        auto refClass = array->klass;
        if (refClass == 0) {
            return;
        }
        auto size = arr->getSize();
        for (uint32_t i = 0; i < size; i++) {
            auto elem = arr->getElem(i);
            if (elem != 0) {
                markClass(reinterpret_cast<Class *>(elem), state);
            }
        }
    }
    void mark(MarkWord::GCState state)
    {
        for (auto &root : roots_) {
            LOG_DEBUG("ObjectHeader " << root, vm_->getLogLevel());
            if (root == 0) {
                LOG_DEBUG("nullptr in root", vm_->getLogLevel());
                continue;
            }
            if (root->getGCState() == state) {
                continue;
            }
            auto classWord = root->getClassWord();
            if (classWord == stringClassWord_) {
                // If we met string from root, we can not access anything else from it
                // just mark string as alive object and move forward
                root->setGCState(state);
                continue;
            }
            auto baseClass = reinterpret_cast<BaseClass *>(classWord);
            if (baseClass->type == ARRAY) {
                markArray(reinterpret_cast<Array *>(root), state);
            }
            if (baseClass->type == DEFAULT) {
                markClass(reinterpret_cast<Class *>(root), state);
            }
        }
    }
    bool isMarked(ObjectHeader *obj)
    {
        auto gcState = obj->getGCState();
        return true ? gcState == MarkWord::GCState::MARKED : false;
    }
    void sweep()
    {
        LOG_DEBUG("[BEFORE SWEEP] Amount of allocations : " << vm_->getAllocator().getAllocations().size(),
                  vm_->getLogLevel());
        auto &allocator = vm_->getAllocator();
        LOG_DEBUG("Start of sweeping", vm_->getLogLevel());
        auto &allocations = allocator.getAllocations();
        std::list<AllocationInfo>::iterator objectInfo = allocations.begin();
        while (objectInfo != allocations.end()) {
            auto object = objectInfo->ptr;
            auto size = objectInfo->size;
            LOG_DEBUG("FOUND IN HEAP : " << std::hex << object << std::dec, vm_->getLogLevel());
            bool is_marked = isMarked(reinterpret_cast<ObjectHeader *>(object));
            if (is_marked) {
                objectInfo++;
            } else {
                objectInfo = allocations.erase(objectInfo);
                LOG_DEBUG("DEAD : " << std::hex << object << std::dec, vm_->getLogLevel());
                allocator.deallocate(object, size);
            }
        }
        LOG_DEBUG("[AFTER SWEEP] Amount of allocations : " << vm_->getAllocator().getAllocations().size(),
                  vm_->getLogLevel());
        LOG_DEBUG("End of sweeping", vm_->getLogLevel());
    }

    std::vector<ObjectHeader *> roots_ {};
    ShrimpVM *vm_;
    ClassWord stringClassWord_;
};

}  // namespace shrimp::runtime::mem

#endif  // RUNTIME_MEMORY_GC_HPP