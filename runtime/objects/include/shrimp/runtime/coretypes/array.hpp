#ifndef RUNTIME_CORETYPES_ARRAY_HPP
#define RUNTIME_CORETYPES_ARRAY_HPP

#include <cstdint>
#include <cstring>
#include <shrimp/runtime/memory/object_header.hpp>
#include <shrimp/runtime/shrimp_vm.hpp>
#include "shrimp/runtime/memory/class_word.hpp"

namespace shrimp::runtime {

class Array : public ObjectHeader {
public:
    static Array *AllocateArrayRef(ClassWord classStruct, uint32_t size, ShrimpVM *vm)
    {
        auto ptr = AllocateArray(size, vm);
        ptr->setClassWord(classStruct);
        return ptr;
    }
    static Array *AllocateArray(uint32_t size, ShrimpVM *vm)
    {
        auto ptr = reinterpret_cast<Array *>(
            vm->getAllocator().allocate(sizeof(ObjectHeader) + sizeof(size) + size * sizeof(uint64_t)));
        if (ptr != nullptr) {
            ptr->setSize(size);
        }
        LOG_INFO("ptr : " << ptr, vm->getLogLevel());
        LOG_INFO("size : " << size, vm->getLogLevel());
        LOG_INFO("data : " << ptr->getData(), vm->getLogLevel());
        return ptr;
    }
    uint64_t getElem(uint32_t pos)
    {
        return data_[pos];
    }
    void setElem(uint64_t value, uint32_t pos)
    {
        data_[pos] = value;
    }
    void setData(uint64_t *data)
    {
        if (data == nullptr) {
            return;
        }
        memcpy(data_, data, size_);
    }
    void setSize(uint32_t size)
    {
        size_ = size;
    }
    uint32_t getSize()
    {
        return size_;
    }
    const uint64_t *getData() const
    {
        return data_;
    }

private:
    uint32_t size_;
    __extension__ uint64_t data_[0];
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_CORETYPES_ARRAY_HPP