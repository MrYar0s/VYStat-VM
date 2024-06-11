#ifndef RUNTIME_MEMORY_OBJECT_HEADER_HPP
#define RUNTIME_MEMORY_OBJECT_HEADER_HPP

#include <shrimp/runtime/memory/mark_word.hpp>
#include <shrimp/runtime/memory/class_word.hpp>

namespace shrimp::runtime {

class ObjectHeader {
public:
    void setClassWord(ClassWord classWord)
    {
        classWord_ = classWord;
    }
    ClassWord getClassWord()
    {
        return classWord_;
    }

private:
    MarkWord markWord_;
    ClassWord classWord_;
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_MEMORY_OBJECT_HEADER_HPP
