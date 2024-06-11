#ifndef RUNTIME_CORETYPES_STRING_HPP
#define RUNTIME_CORETYPES_STRING_HPP

#include <cstdint>
#include <cstring>
#include <shrimp/runtime/memory/object_header.hpp>
#include <shrimp/runtime/shrimp_vm.hpp>

namespace shrimp::runtime {

class String : public ObjectHeader {
public:
    static String *SubStr(String *strObj, uint32_t pos, uint32_t size, ShrimpVM *vm)
    {
        std::string str = std::string(strObj->getData()).substr(pos, size);
        return AllocateString(size, str.data(), vm);
    }
    static String *ConcatStrings(String *strObj0, String *strObj1, ShrimpVM *vm)
    {
        uint32_t len = strObj0->getSize() + strObj1->getSize();
        std::string str = std::string(strObj0->getData()) + std::string(strObj1->getData());
        return AllocateString(len, str.data(), vm);
    }
    static String *AllocateString(uint32_t size, const char *data, ShrimpVM *vm)
    {
        auto ptr = reinterpret_cast<String *>(
            vm->getAllocator().allocate(sizeof(ObjectHeader) + sizeof(size) + sizeof(hashCode_) + size));
        if (ptr != nullptr) {
            ptr->setSize(size);
            ptr->setHashCode(0xd09c);
            ptr->setData(data);
            ptr->setClassWord(0);
        }
        return ptr;
    }
    void setData(const char *data)
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
    void setHashCode(uint32_t hashCode)
    {
        hashCode_ = hashCode;
    }
    uint32_t getSize()
    {
        return size_;
    }
    uint32_t getHashCode()
    {
        return hashCode_;
    }
    const char *getData() const
    {
        return data_;
    }

private:
    uint32_t size_;
    uint32_t hashCode_;
    __extension__ char data_[0];
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_CORETYPES_STRING_HPP