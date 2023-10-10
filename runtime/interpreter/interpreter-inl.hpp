#ifndef RUNTIME_INTERPRETER_INTERPRETER_INL_HPP
#define RUNTIME_INTERPRETER_INTERPRETER_INL_HPP

#include "frame.hpp"
#include "interpreter.hpp"
#include "bytecode_inst.hpp"
#include "bitops.hpp"
#include "types.hpp"
#include <iostream>
#include "intrinsics.hpp"

namespace shrimp::interpreter {

int handleNop(const InstType *pc, Frame *frame);
int handleMov(const InstType *pc, Frame *frame);
int handleMovImmF(const InstType *pc, Frame *frame);
int handleMovImmI(const InstType *pc, Frame *frame);
int handleLda(const InstType *pc, Frame *frame);
int handleLdaImmF(const InstType *pc, Frame *frame);
int handleLdaImmI(const InstType *pc, Frame *frame);
int handleSta(const InstType *pc, Frame *frame);
int handleAddI(const InstType *pc, Frame *frame);
int handleAddF(const InstType *pc, Frame *frame);
int handleSubI(const InstType *pc, Frame *frame);
int handleSubF(const InstType *pc, Frame *frame);
int handleDivI(const InstType *pc, Frame *frame);
int handleDivF(const InstType *pc, Frame *frame);
int HandleNegI(const InstType *pc, Frame *frame);
int HandleNegF(const InstType *pc, Frame *frame);
int handleMulI(const InstType *pc, Frame *frame);
int handleMulF(const InstType *pc, Frame *frame);
int handleReturn(const InstType *pc, Frame *frame);
int handleIntrinsic(const InstType *pc, Frame *frame);

static const size_t DISPATCH_LEN = 20;
static std::array<int (*)(const InstType *pc, Frame *frame), DISPATCH_LEN> dispatch_table {
    &handleNop,  &handleMov,  &handleMovImmF, &handleMovImmI, &handleLda,    &handleLdaImmF,  &handleLdaImmI,
    &handleSta,  &handleAddI, &handleAddF,    &handleSubI,    &handleSubF,   &handleDivI,     &handleDivF,
    &HandleNegI, &HandleNegF, &handleMulI,    &handleMulF,    &handleReturn, &handleIntrinsic};

static InstType OPCODE_MASK = 0xff;

inline uint64_t getOpcode(const InstType *pc)
{
    return *pc & OPCODE_MASK;
}

inline int handleNop(const InstType *pc, Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "nop" << std::endl;
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMov(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_R8>(pc);
    auto dst_reg_num = inst.getRegNum<0>();
    auto src_reg_num = inst.getRegNum<1>();
    std::cout << "LOG_INFO: "
              << "mov "
              << "r" << (size_t)dst_reg_num << " r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    frame->setReg(src_reg.getValue(), dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMovImmF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_IMM32>(pc);
    auto dst_reg_num = inst.getRegNum();
    auto val = getValue<float>(inst.getImm());
    std::cout << "LOG_INFO: "
              << "mov.immf "
              << "r" << (size_t)dst_reg_num << " " << val << std::endl;
    auto res = castToWritable<float>(val);
    frame->setReg(res, dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMovImmI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_IMM32>(pc);
    auto dst_reg_num = inst.getRegNum();
    auto val = getValue<int>(inst.getImm());
    std::cout << "LOG_INFO: "
              << "mov.immi "
              << "r" << (size_t)dst_reg_num << " " << val << std::endl;
    auto res = castToWritable<int>(val);
    frame->setReg(res, dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleLda(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "lda "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    frame->setAcc(src_reg.getValue());
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleLdaImmF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::IMM32>(pc);
    auto val = getValue<float>(inst.getImm());
    std::cout << "LOG_INFO: "
              << "lda.immf " << val << std::endl;
    auto res = castToWritable<float>(val);
    frame->setAcc(res);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleLdaImmI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::IMM32>(pc);
    auto val = getValue<int>(inst.getImm());
    std::cout << "LOG_INFO: "
              << "lda.immi " << val << std::endl;
    auto res = castToWritable<int>(val);
    frame->setAcc(res);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleSta(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto dst_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "sta "
              << "r" << (size_t)dst_reg_num << std::endl;
    frame->setReg(acc.getValue(), dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleAddI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "addi "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    int acc_val = getValue<int>(acc.getValue());
    int src_val = getValue<int>(src_reg.getValue());
    int res = acc_val + src_val;
    uint64_t res_u64 = castToWritable<int>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleAddF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "addf "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    float src_val = getValue<float>(src_reg.getValue());
    float res = acc_val + src_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

// Sub from register value from accumulator
inline int handleSubI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "subi "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    int acc_val = getValue<int>(acc.getValue());
    int src_val = getValue<int>(src_reg.getValue());
    int res = acc_val - src_val;
    uint64_t res_u64 = castToWritable<int>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleSubF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "subf "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    float src_val = getValue<float>(src_reg.getValue());
    float res = acc_val + src_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleDivI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "divi "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    int src_val = getValue<int>(src_reg.getValue());
    float res = acc_val / src_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleDivF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "divf "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    float src_val = getValue<float>(src_reg.getValue());
    float res = acc_val / src_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int HandleNegI(const InstType *pc, Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "negi" << std::endl;
    auto acc = frame->getAcc();
    int acc_val = getValue<int>(acc.getValue());
    int res = -acc_val;
    uint64_t res_u64 = castToWritable<int>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int HandleNegF(const InstType *pc, Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "negf" << std::endl;
    auto acc = frame->getAcc();
    float acc_val = getValue<int>(acc.getValue());
    float res = -acc_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMulI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "muli "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    int acc_val = getValue<int>(acc.getValue());
    int src_val = getValue<int>(src_reg.getValue());
    int res = acc_val * src_val;
    uint64_t res_u64 = castToWritable<int>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMulF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "mulf "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    float src_val = getValue<float>(src_reg.getValue());
    float res = acc_val * src_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleIntrinsic(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_R8_ID16>(pc);
    [[maybe_unused]] auto first_reg_num = inst.getRegNum<0>();
    [[maybe_unused]] auto second_reg_num = inst.getRegNum<1>();
    auto id = inst.getId();
    auto intrinsic_type = static_cast<IntrinsicType>(id);
    switch (intrinsic_type) {
        case IntrinsicType::PrintI: {
            std::cout << "LOG_INFO: " << "intrinsic printi, " << "r" << (size_t)first_reg_num << std::endl;
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = getValue<int>(reg.getValue());
            PrintI(first_reg);
            break;
        }
        case IntrinsicType::PrintF: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic printf, " << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<float>(reg.getValue());
            PrintF(first_reg);
            break;
        }
        case IntrinsicType::ScanI: {
            std::cout << "LOG_INFO: " << "intrinsic scani" << std::endl;
            int res = ScanI();
            uint64_t res_u64 = castToWritable<int>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::ScanF: {
            std::cout << "LOG_INFO: " << "intrinsic scanf" << std::endl;
            float res = ScanF();
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::SinF: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic sinf, " << "r" << (size_t)first_reg_num << std::endl;
            float first_reg = getValue<float>(reg.getValue());
            float res = SinF(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::SinI: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic sini, " << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<int>(reg.getValue());
            float res = SinI(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::CosF: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic cosf, " << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<float>(reg.getValue());
            float res = CosF(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::CosI: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic cosi, " << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<int>(reg.getValue());
            float res = CosI(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::SqrtF: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic sqrtf, " << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<float>(reg.getValue());
            float res = SqrtF(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicType::SqrtI: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: " << "intrinsic sqrti, " << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<int>(reg.getValue());
            float res = SqrtF(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        default: {
            std::cerr << "Unsupported intrinsic" << std::endl;
            std::abort();
        }
    }
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleReturn([[maybe_unused]] const InstType *pc, [[maybe_unused]] Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "return" << std::endl;
    return 0;
}

}  // namespace shrimp::interpreter

#endif  // RUNTIME_INTERPRETER_INTERPRETER_INL_HPP