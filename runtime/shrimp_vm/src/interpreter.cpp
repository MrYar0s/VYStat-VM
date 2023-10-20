#include <cassert>
#include <iostream>
#include <array>
#include <sstream>

#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/runtime/shrimp_vm/instr.gen.hpp>
#include <shrimp/runtime/shrimp_vm/intrinsics.hpp>
#include <shrimp/runtime/shrimp_vm/interpreter.hpp>

namespace shrimp::runtime::interpreter {

std::string Instr<InstrOpcode::INTRINSIC>::toString() const
{
    std::stringstream out;

    switch (static_cast<IntrinsicCode>(getIntrinsicCode())) {
        case IntrinsicCode::PRINT_I32:
            out << "PRINT.I32, R" << getIntrinsicArg0();
            break;

        case IntrinsicCode::PRINT_F:
            out << "PRINT.F, R" << getIntrinsicArg0();
            break;

        case IntrinsicCode::SCAN_I32:
            out << "SCAN.I32";
            break;

        case IntrinsicCode::SCAN_F:
            out << "SCAN.F";
            break;

        case IntrinsicCode::COS:
            out << "COS, R" << getIntrinsicArg0();
            break;

        case IntrinsicCode::SIN:
            out << "SIN, R" << getIntrinsicArg0();
            break;

        case IntrinsicCode::SQRT:
            out << "SQRT, R" << getIntrinsicArg0();
            break;

        default:
            assert(0);
    }

    return out.str();
}

namespace {

int handleNop(const Byte *pc, Frame *frame);

int handleMov(const Byte *pc, Frame *frame);
int handleMovImmI32(const Byte *pc, Frame *frame);
int handleMovImmF(const Byte *pc, Frame *frame);

int handleLda(const Byte *pc, Frame *frame);
int handleLdaImmI32(const Byte *pc, Frame *frame);
int handleLdaImmF(const Byte *pc, Frame *frame);

int handleSta(const Byte *pc, Frame *frame);

int handleAddI32(const Byte *pc, Frame *frame);
int handleAddF(const Byte *pc, Frame *frame);

int handleSubI32(const Byte *pc, Frame *frame);
int handleSubF(const Byte *pc, Frame *frame);

int handleDivI32(const Byte *pc, Frame *frame);
int handleDivF(const Byte *pc, Frame *frame);

int handleMulI32(const Byte *pc, Frame *frame);
int handleMulF(const Byte *pc, Frame *frame);

int handleRet(const Byte *pc, Frame *frame);
int handleIntrinsic(const Byte *pc, Frame *frame);

static constexpr size_t DISPATCH_LEN = 21;
static constexpr std::array<int (*)(const Byte *pc, Frame *frame), DISPATCH_LEN> dispatch_table {
    nullptr,        &handleNop, &handleMov,    &handleMovImmI32, &handleMovImmF, &handleLda,  &handleLdaImmI32,
    &handleLdaImmF, &handleSta, &handleAddI32, &handleAddF,      &handleSubI32,  &handleSubF, &handleDivI32,
    &handleDivF,    nullptr,    nullptr,       &handleMulI32,    &handleMulF,    &handleRet,  &handleIntrinsic};

static constexpr Byte OPCODE_MASK = 0xff;

Byte getOpcode(const Byte *pc)
{
    return *pc & OPCODE_MASK;
}

int handleNop(const Byte *pc, Frame *frame)
{
    Instr<InstrOpcode::NOP> instr {};

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMov(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV>(pc);
    auto dst_reg_num = instr.getRd();
    auto src_reg_num = instr.getRs();

    auto src_reg = frame->getReg(src_reg_num);
    frame->setReg(src_reg.getValue(), dst_reg_num);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMovImmF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_F>(pc);
    auto dst_reg_num = instr.getRd();
    auto val = bit::getValue<float>(instr.getImmF());

    auto res = bit::castToWritable<float>(val);
    frame->setReg(res, dst_reg_num);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMovImmI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_I32>(pc);
    auto dst_reg_num = instr.getRd();
    auto val = bit::getValue<int>(instr.getImmI32());

    auto res = bit::castToWritable<int>(val);
    frame->setReg(res, dst_reg_num);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLda(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA>(pc);
    auto src_reg_num = instr.getRs();

    auto src_reg = frame->getReg(src_reg_num);
    frame->setAcc(src_reg.getValue());

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLdaImmF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_F>(pc);
    auto val = bit::getValue<float>(instr.getImmF());

    auto res = bit::castToWritable<float>(val);
    frame->setAcc(res);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLdaImmI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_I32>(pc);
    auto val = bit::getValue<int>(instr.getImmI32());

    auto res = bit::castToWritable<int>(val);
    frame->setAcc(res);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleSta(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::STA>(pc);
    auto dst_reg_num = instr.getRd();

    auto acc = frame->getAcc();
    frame->setReg(acc.getValue(), dst_reg_num);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleAddI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::ADD_I32>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    int acc_val = bit::getValue<int>(acc.getValue());
    int src_val = bit::getValue<int>(src_reg.getValue());
    int res = acc_val + src_val;
    uint64_t res_u64 = bit::castToWritable<int>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleAddF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::ADD_F>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = bit::getValue<float>(acc.getValue());
    float src_val = bit::getValue<float>(src_reg.getValue());
    float res = acc_val + src_val;
    uint64_t res_u64 = bit::castToWritable<float>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

// Sub from register value from accumulator
int handleSubI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::SUB_I32>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    int acc_val = bit::getValue<int>(acc.getValue());
    int src_val = bit::getValue<int>(src_reg.getValue());
    int res = acc_val - src_val;
    uint64_t res_u64 = bit::castToWritable<int>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleSubF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::SUB_F>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = bit::getValue<float>(acc.getValue());
    float src_val = bit::getValue<float>(src_reg.getValue());
    float res = acc_val - src_val;
    uint64_t res_u64 = bit::castToWritable<float>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleDivI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::DIV_I32>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = bit::getValue<float>(acc.getValue());
    int src_val = bit::getValue<int>(src_reg.getValue());
    float res = acc_val / src_val;
    uint64_t res_u64 = bit::castToWritable<float>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleDivF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::DIV_F>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = bit::getValue<float>(acc.getValue());
    float src_val = bit::getValue<float>(src_reg.getValue());
    float res = acc_val / src_val;
    uint64_t res_u64 = bit::castToWritable<float>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMulI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MUL_I32>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    int acc_val = bit::getValue<int>(acc.getValue());
    int src_val = bit::getValue<int>(src_reg.getValue());
    int res = acc_val * src_val;
    uint64_t res_u64 = bit::castToWritable<int>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMulF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MUL_F>(pc);
    auto src_reg_num = instr.getRs();

    auto acc = frame->getAcc();
    auto src_reg = frame->getReg(src_reg_num);
    float acc_val = bit::getValue<float>(acc.getValue());
    float src_val = bit::getValue<float>(src_reg.getValue());
    float res = acc_val * src_val;
    uint64_t res_u64 = bit::castToWritable<float>(res);
    frame->setAcc(res_u64);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleIntrinsic(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::INTRINSIC>(pc);

    auto intrinsic_type = static_cast<IntrinsicCode>(instr.getIntrinsicCode());

    [[maybe_unused]] auto first_reg_num = instr.getIntrinsicArg0();
    [[maybe_unused]] auto second_reg_num = instr.getIntrinsicArg1();

    std::cout << "LOG: " << instr.toString() << std::endl;

    switch (intrinsic_type) {
        case IntrinsicCode::PRINT_I32: {
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = bit::getValue<int>(reg.getValue());

            intrinsics::PrintI(first_reg);
            break;
        }
        case IntrinsicCode::PRINT_F: {
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = bit::getValue<float>(reg.getValue());

            intrinsics::PrintF(first_reg);
            break;
        }
        case IntrinsicCode::SCAN_I32: {
            int res = intrinsics::ScanI();
            uint64_t res_u64 = bit::castToWritable<int>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SCAN_F: {
            float res = intrinsics::ScanF();
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SIN: {
            auto reg = frame->getReg(first_reg_num);
            float first_reg = bit::getValue<float>(reg.getValue());

            float res = intrinsics::SinF(first_reg);
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::COS: {
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = bit::getValue<float>(reg.getValue());

            float res = intrinsics::CosF(first_reg);
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SQRT: {
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = bit::getValue<float>(reg.getValue());

            if (first_reg < 0.0) {
                throw std::runtime_error("Negative value under square root");
            }
            float res = intrinsics::SqrtF(first_reg);
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        default: {
            std::cerr << "Unsupported intrinsic" << std::endl;
            std::abort();
        }
    }
    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleRet([[maybe_unused]] const Byte *pc, [[maybe_unused]] Frame *frame)
{
    Instr<InstrOpcode::RET> instr {};

    std::cout << "LOG: " << instr.toString() << std::endl;

    return 0;
}

}  // namespace

int runImpl(const Byte *pc, Frame *frame)
{
    dispatch_table[getOpcode(pc)](pc, frame);
    return 0;
}

}  // namespace shrimp::runtime::interpreter
