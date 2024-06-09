#ifndef RUNTIME_MEMORY_MARK_WORD_HPP
#define RUNTIME_MEMORY_MARK_WORD_HPP

#include <cstdint>
#include <shrimp/common/logger.hpp>

namespace shrimp::runtime {

class MarkWord {
public:
    enum class ObjectState {
        UNLOCKED,
        HASHED,
        GC,
        INVALID,
    };

    ObjectState GetState() const
    {
        switch ((value_ >> STATUS_SHIFT) & STATUS_MASK) {
            case STATUS_UNLOCKED:
                return ObjectState::UNLOCKED;
            case STATUS_HASHED:
                return ObjectState::HASHED;
            case STATUS_GC:
                return ObjectState::GC;
            default:
                LOG_ERROR("Undefined object state", ERROR);
                return ObjectState::INVALID;
        }
    }

private:
    enum MarkWordUtils : uint32_t {
        MARK_WORD_SIZE = 32,
        STATUS_SIZE = 2,
        GC_STATUS_SIZE = 1,
        READB_STATUS_SIZE = 1,  // For future support of multithreding
        HASH_STATUS_SIZE = 28,
        FORWARDING_ADDR_SIZE = 30,

        // Unlocked state masks and shifts
        UNLOCKED_STATE_SHIFT = MARK_WORD_SIZE - HASH_STATUS_SIZE,
        UNLOCKED_STATE_MASK = (1 << HASH_STATUS_SIZE) - 1,
        UNLOCKED_STATE_MASK_IN_PLACE = UNLOCKED_STATE_MASK << UNLOCKED_STATE_SHIFT,

        // Hashed state masks and shifts
        HASH_STATE_SHIFT = UNLOCKED_STATE_SHIFT,
        HASH_STATE_MASK = UNLOCKED_STATE_MASK,
        HASH_STATE_MASK_IN_PLACE = UNLOCKED_STATE_MASK_IN_PLACE,

        // Read barrier status masks and shifts
        READB_STATUS_SHIFT = UNLOCKED_STATE_SHIFT - READB_STATUS_SIZE,
        READB_STATUS_MASK = (1 << READB_STATUS_SIZE) - 1,
        READB_STATUS_MASK_IN_PLACE = READB_STATUS_MASK << READB_STATUS_SHIFT,

        // GC status masks and shifts
        GC_STATUS_SHIFT = READB_STATUS_SHIFT - GC_STATUS_SIZE,
        GC_STATUS_MASK = (1 << GC_STATUS_SHIFT) - 1,
        GC_STATUS_MASK_IN_PLACE = GC_STATUS_MASK << GC_STATUS_SHIFT,

        // Status masks and shifts
        STATUS_SHIFT = GC_STATUS_SHIFT - STATUS_SIZE,
        STATUS_MASK = (1 << STATUS_SIZE) - 1,
        STATUS_MASK_IN_PLACE = STATUS_MASK << STATUS_SHIFT,

        // Forwarding address masks and shifts
        FORWARDING_ADDR_SHIFT = MARK_WORD_SIZE - FORWARDING_ADDR_SIZE,
        FORWARDING_ADDR_MASK = (1 << FORWARDING_ADDR_SIZE) - 1,
        FORWARDING_ADDR_MASK_IN_PLACE = FORWARDING_ADDR_MASK << FORWARDING_ADDR_SHIFT,

        // Marks of object states
        STATUS_UNLOCKED = 0,
        STATUS_HASHED = 2,
        STATUS_GC = 3,
    };

private:
    MarkWordUtils value_ {0};
};

}  // namespace shrimp::runtime

#endif  // RUNTIME_MEMORY_MARK_WORD_HPP
