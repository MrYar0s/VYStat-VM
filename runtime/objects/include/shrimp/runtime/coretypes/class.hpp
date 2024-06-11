#ifndef RUNTIME_CORETYPES_CLASS_HPP
#define RUNTIME_CORETYPES_CLASS_HPP

#include <cstdint>
#include <cstring>
#include <shrimp/runtime/memory/object_header.hpp>
#include <shrimp/runtime/shrimp_vm.hpp>
#include "shrimp/common/logger.hpp"
#include "shrimp/common/types.hpp"
#include "shrimp/runtime/memory/class_word.hpp"

namespace shrimp::runtime {

class Class : public ObjectHeader {
public:
    static Class *AllocateClassRef(ClassWord classStruct, uint32_t size, ShrimpVM *vm)
    {
        auto ptr = AllocateClass(size, vm);
        ptr->setClassWord(classStruct);
        return ptr;
    }
    static Class *AllocateClass(uint32_t size, ShrimpVM *vm)
    {
        auto ptr = reinterpret_cast<Class *>(vm->getAllocator().allocate(sizeof(ObjectHeader) + size));
        if (ptr != nullptr) {
            memset(ptr->data_, 0, size);
        }
        LOG_INFO("ptr : " << ptr, vm->getLogLevel());
        LOG_INFO("size : " << size, vm->getLogLevel());
        return ptr;
    }
    [[nodiscard]] uint64_t getField(const RuntimeField &field)
    {
        auto offset = field.offset;
        auto size = field.size;
        uint64_t ld_tmp = 0;
        memcpy(&ld_tmp, data_ + offset, size);
        return ld_tmp;
    }
    void setField(const RuntimeField &field, uint64_t value)
    {
        auto offset = field.offset;
        auto size = field.size;
        memcpy(data_ + offset, &value, size);
    }
    void setData(uint64_t *data, uint32_t size)
    {
        if (data == nullptr) {
            return;
        }
        memcpy(data_, data, size);
    }
    const uint64_t *getData() const
    {
        return data_;
    }

private:
    __extension__ uint64_t data_[0];
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_CORETYPES_CLASS_HPP