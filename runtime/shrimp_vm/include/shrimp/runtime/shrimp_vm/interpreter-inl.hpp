#ifndef RUNTIME_SHRIMP_VM_INTERPRETER_INL_HPP
#define RUNTIME_SHRIMP_VM_INTERPRETER_INL_HPP

#include <iostream>
#include <array>
#include <stdexcept>

#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>

#include <shrimp/runtime/frame.hpp>
#include <shrimp/runtime/bytecode_inst.hpp>

#include <shrimp/runtime/shrimp_vm/interpreter.hpp>
#include <shrimp/runtime/shrimp_vm/intrinsics.hpp>

namespace shrimp::interpreter {

int handleNop(const InstType *pc, Frame *frame);

int handleMov(const InstType *pc, Frame *frame);
int handleMovImmI32(const InstType *pc, Frame *frame);
int handleMovImmF(const InstType *pc, Frame *frame);

int handleLda(const InstType *pc, Frame *frame);
int handleLdaImmI32(const InstType *pc, Frame *frame);
int handleLdaImmF(const InstType *pc, Frame *frame);

int handleSta(const InstType *pc, Frame *frame);

int handleAddI32(const InstType *pc, Frame *frame);
int handleAddF(const InstType *pc, Frame *frame);

int handleSubI32(const InstType *pc, Frame *frame);
int handleSubF(const InstType *pc, Frame *frame);

int handleDivI32(const InstType *pc, Frame *frame);
int handleDivF(const InstType *pc, Frame *frame);

int HandleNegI32(const InstType *pc, Frame *frame);
int HandleNegF(const InstType *pc, Frame *frame);

int handleMulI32(const InstType *pc, Frame *frame);
int handleMulF(const InstType *pc, Frame *frame);

int handleRet(const InstType *pc, Frame *frame);
int handleIntrinsic(const InstType *pc, Frame *frame);

static const size_t DISPATCH_LEN = 21;
static std::array<int (*)(const InstType *pc, Frame *frame), DISPATCH_LEN> dispatch_table {
    nullptr,        &handleNop,    &handleMov,    &handleMovImmI32, &handleMovImmF, &handleLda,  &handleLdaImmI32,
    &handleLdaImmF, &handleSta,    &handleAddI32, &handleAddF,      &handleSubI32,  &handleSubF, &handleDivI32,
    &handleDivF,    &HandleNegI32, &HandleNegF,   &handleMulI32,    &handleMulF,    &handleRet,  &handleIntrinsic};

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
              << "mov.imm.f "
              << "r" << (size_t)dst_reg_num << " " << val << std::endl;
    auto res = castToWritable<float>(val);
    frame->setReg(res, dst_reg_num);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMovImmI32(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8_IMM32>(pc);
    auto dst_reg_num = inst.getRegNum();
    auto val = getValue<int>(inst.getImm());
    std::cout << "LOG_INFO: "
              << "mov.imm.i32 "
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
              << "lda.imm.f " << val << std::endl;
    auto res = castToWritable<float>(val);
    frame->setAcc(res);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleLdaImmI32(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::IMM32>(pc);
    auto val = getValue<int>(inst.getImm());
    std::cout << "LOG_INFO: "
              << "lda.imm.i32 " << val << std::endl;
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

inline int handleAddI32(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "add.i32 "
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
              << "add.f "
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
inline int handleSubI32(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "sub.i32 "
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
              << "sub.f "
              << "r" << (size_t)src_reg_num << std::endl;
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = getValue<float>(acc.getValue());
    float src_val = getValue<float>(src_reg.getValue());
    float res = acc_val - src_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleDivI32(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "div.i32 "
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
              << "div.f "
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

inline int HandleNegI32(const InstType *pc, Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "neg.i32" << std::endl;
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
              << "neg.f" << std::endl;
    auto acc = frame->getAcc();
    float acc_val = getValue<float>(acc.getValue());
    float res = -acc_val;
    uint64_t res_u64 = castToWritable<float>(res);
    frame->setAcc(res_u64);
    pc++;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

inline int handleMulI32(const InstType *pc, Frame *frame)
{
    auto inst = BytecodeInstruction<Format::R8>(pc);
    auto acc = frame->getAcc();
    auto src_reg_num = inst.getRegNum();
    std::cout << "LOG_INFO: "
              << "mul.i32 "
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
              << "mul.f "
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
    auto intrinsic_type = static_cast<IntrinsicCode>(id);
    switch (intrinsic_type) {
        case IntrinsicCode::PRINT_I32: {
            std::cout << "LOG_INFO: "
                      << "intrinsic print.i32, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = getValue<int>(reg.getValue());
            PrintI(first_reg);
            break;
        }
        case IntrinsicCode::PRINT_F: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic print.f, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<float>(reg.getValue());
            PrintF(first_reg);
            break;
        }
        case IntrinsicCode::SCAN_I32: {
            std::cout << "LOG_INFO: "
                      << "intrinsic scan.i32" << std::endl;
            int res = ScanI();
            uint64_t res_u64 = castToWritable<int>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SCAN_F: {
            std::cout << "LOG_INFO: "
                      << "intrinsic scan.f" << std::endl;
            float res = ScanF();
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SIN: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic sin, "
                      << "r" << (size_t)first_reg_num << std::endl;
            float first_reg = getValue<float>(reg.getValue());
            float res = SinF(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::COS: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic cos, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<float>(reg.getValue());
            float res = CosF(first_reg);
            uint64_t res_u64 = castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SQRT: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic sqrt, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = getValue<float>(reg.getValue());
            if (first_reg < 0.0) {
                throw std::runtime_error("Negative value under square root");
            }
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

inline int handleRet([[maybe_unused]] const InstType *pc, [[maybe_unused]] Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "ret" << std::endl;
    return 0;
}

}  // namespace shrimp::interpreter

#endif  // RUNTIME_SHRIMP_VM_INTERPRETER_INL_HPP
