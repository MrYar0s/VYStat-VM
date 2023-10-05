#ifndef RUNTIME_BYTECODE_INST_HPP
#define RUNTIME_BYTECODE_INST_HPP

#include "bitops.hpp"
#include "types.hpp"
#include <array>
#include <cstdlib>
#include <cstdint>

namespace shrimp {

enum class Format : uint8_t { R8, R8_R8, R8_IMM32, IMM32 };

template <enum Format>
class BytecodeInstruction {
public:
    explicit BytecodeInstruction(const InstType *pc);

    template <size_t idx>
    uint8_t getRegNum();

    uint8_t getRegNum();
    uint64_t getImm();

private:
    InstType raw_inst;
};

template <>
class BytecodeInstruction<Format::R8> {
public:
    explicit BytecodeInstruction(const InstType *pc)
    {
        raw_inst_ = *pc;
        reg_ = applyMask<0xff>(applyRightShift<56>(raw_inst_));
    }
    uint8_t getRegNum()
    {
        return reg_;
    }

private:
    uint8_t reg_;
    uint64_t raw_inst_;
};

template <>
class BytecodeInstruction<Format::R8_R8> {
public:
    explicit BytecodeInstruction(const InstType *pc)
    {
        raw_inst_ = *pc;
        reg_[0] = applyMask<0xff>(applyRightShift<56>(raw_inst_));
        reg_[1] = applyMask<0xff>(applyRightShift<48>(raw_inst_));
    }
    template <size_t idx>
    uint8_t getRegNum()
    {
        return reg_[idx];
    }

private:
    std::array<uint8_t, 2> reg_;
    uint64_t raw_inst_;
};

template <>
class BytecodeInstruction<Format::R8_IMM32> {
public:
    explicit BytecodeInstruction(const InstType *pc)
    {
        raw_inst_ = *pc;
        reg_ = applyMask<0xff>(applyRightShift<56>(raw_inst_));
        imm_ = applyMask<0xffffffff>(applyRightShift<24>(raw_inst_));
    }
    uint8_t getRegNum()
    {
        return reg_;
    }
    uint32_t getImm()
    {
        return imm_;
    }

private:
    uint8_t reg_;
    uint32_t imm_;
    uint64_t raw_inst_;
};

template <>
class BytecodeInstruction<Format::IMM32> {
public:
    explicit BytecodeInstruction(const InstType *pc)
    {
        raw_inst_ = *pc;
        imm_ = applyMask<0xffffffff>(applyRightShift<32>(raw_inst_));
    }
    uint32_t getImm()
    {
        return imm_;
    }

private:
    uint32_t imm_;
    uint64_t raw_inst_;
};

}  // namespace shrimp

#endif  // RUNTIME_BYTECODE_INST_HPP