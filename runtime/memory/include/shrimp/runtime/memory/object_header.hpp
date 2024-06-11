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
    auto getState() const
    {
        return markWord_.getState();
    }
    auto getGCState() const
    {
        return markWord_.getGCState();
    }
    void setGCState(MarkWord::GCState state)
    {
        switch (state) {
            case MarkWord::GCState::UNMARKED:
                markWord_.setGCUnmarked();
                break;
            case MarkWord::GCState::MARKED:
                markWord_.setGCMarked();
                break;
            default:
                LOG_ERROR("Undefined GC state in setter", ERROR);
                break;
        }
    }

private:
    MarkWord markWord_;
    ClassWord classWord_;
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_MEMORY_OBJECT_HEADER_HPP
