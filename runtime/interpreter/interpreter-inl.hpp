#ifndef RUNTIME_INTERPRETER_INTERPRETER_INL_HPP
#define RUNTIME_INTERPRETER_INTERPRETER_INL_HPP

#include "interpreter.hpp"
#include "bytecode_inst.hpp"
#include "bitops.hpp"
#include <iostream>

namespace shrimp::interpreter {

int handleNop(const InstType *pc, Frame *frame);
int handleMov(const InstType *pc, Frame *frame);
int handleMovImm(const InstType *pc, Frame *frame);
int handleLda(const InstType *pc, Frame *frame);
int handleLdaImm(const InstType *pc, Frame *frame);
int handleSta(const InstType *pc, Frame *frame);
int handleAddI(const InstType *pc, Frame *frame);
int handleAddL(const InstType *pc, Frame *frame);
int handleAddF(const InstType *pc, Frame *frame);
int handleAddD(const InstType *pc, Frame *frame);
int handleReturn(const InstType *pc, Frame *frame);

static const size_t DISPATCH_LEN = 11;
static std::array<int (*)(const InstType *pc, Frame *frame), DISPATCH_LEN> dispatch_table {
    &handleNop,  &handleMov,  &handleMovImm, &handleLda,  &handleLdaImm, &handleSta,
    &handleAddI, &handleAddL, &handleAddF,   &handleAddD, &handleReturn};

static InstType OPCODE_MASK = 0xff;

inline uint64_t getOpcode(const InstType *pc)
{
    return *pc & OPCODE_MASK;
}

inline int handleNop(const InstType *pc, Frame *frame)
{
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMov(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_R8>(pc);
    auto dst_reg_num = inst.getRegNum<0>();
    auto src_reg_num = inst.getRegNum<1>();
    auto src_reg = frame->getReg(src_reg_num);
    frame->setReg(src_reg.getValue(), dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMovImm(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_IMM32>(pc);
    auto dst_reg_num = inst.getRegNum();
    auto imm = inst.getImm();
    frame->setReg(imm, dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleLda(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_IMM32>(pc);
    auto src_reg_num = inst.getRegNum();
    auto src_reg = frame->getReg(src_reg_num);
    frame->setAcc(src_reg.getValue());
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleLdaImm(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::IMM32>(pc);
    auto imm = inst.getImm();
    frame->setAcc(imm);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleSta(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto dst_reg_num = inst.getRegNum();
    frame->setReg(acc.getValue(), dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleAddI(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto dst_reg_num = inst.getRegNum();
    auto dst_reg = frame->getReg(dst_reg_num);
    int acc_val = getValue<int>(acc.getValue());
    int dst_val = getValue<int>(dst_reg.getValue());
    int res = acc_val + dst_val;
    uint64_t res_u64 = castToWritable<int>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleAddL(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto dst_reg_num = inst.getRegNum();
    auto dst_reg = frame->getReg(dst_reg_num);
    long int acc_val = getValue<long int>(acc.getValue());
    long int dst_val = getValue<long int>(dst_reg.getValue());
    long int res = acc_val + dst_val;
    uint64_t res_u64 = castToWritable<long int>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleAddF(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto dst_reg_num = inst.getRegNum();
    auto dst_reg = frame->getReg(dst_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    float dst_val = getValue<float>(dst_reg.getValue());
    float res = acc_val + dst_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleAddD(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto dst_reg_num = inst.getRegNum();
    auto dst_reg = frame->getReg(dst_reg_num);
    double acc_val = getValue<double>(acc.getValue());
    double dst_val = getValue<double>(dst_reg.getValue());
    double res = acc_val + dst_val;
    uint64_t res_u64 = castToWritable<double>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleReturn([[maybe_unused]] const InstType *pc, [[maybe_unused]] Frame *frame)
{
    return 0;
}

}  // namespace shrimp::interpreter

#endif  // RUNTIME_INTERPRETER_INTERPRETER_INL_HPP