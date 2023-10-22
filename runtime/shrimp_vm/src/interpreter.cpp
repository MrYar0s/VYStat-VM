#include <cassert>
#include <iostream>
#include <array>
#include <sstream>

#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/runtime/shrimp_vm/intrinsics.hpp>
#include <shrimp/runtime/shrimp_vm/interpreter.hpp>

#include <shrimp/runtime/shrimp_vm/instr.gen.hpp>
#include <shrimp/runtime/shrimp_vm/dispatch_table.gen.hpp>

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

static constexpr Byte OPCODE_MASK = 0xff;

Byte getOpcode(const Byte *pc)
{
    return *pc & OPCODE_MASK;
}

int handleNop(const Byte *pc, Frame *frame)
{
    Instr<InstrOpcode::NOP> instr {pc};

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMov(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV>(pc);
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto res = frame->getReg(rs_idx).getValue();
    frame->setReg(res, rd_idx);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMovImmI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_I32>(pc);
    auto rd_idx = instr.getRd();
    auto imm_i32 = bit::getValue<int32_t>(instr.getImmI32());

    frame->setReg(imm_i32, rd_idx);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMovImmF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_F>(pc);
    auto rd_idx = instr.getRd();
    auto imm_f = bit::getValue<float>(instr.getImmF());

    frame->setReg(bit::castToWritable(imm_f), rd_idx);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLda(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA>(pc);
    auto rs_idx = instr.getRs();

    auto res = frame->getReg(rs_idx).getValue();
    frame->setAcc(res);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLdaImmI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_I32>(pc);
    auto imm_i32 = bit::getValue<int32_t>(instr.getImmI32());

    frame->setAcc(imm_i32);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLdaImmF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_F>(pc);
    auto imm_f = bit::getValue<float>(instr.getImmF());

    frame->setAcc(bit::castToWritable(imm_f));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleSta(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::STA>(pc);
    auto rd_idx = instr.getRd();

    auto res = frame->getAcc().getValue();
    frame->setReg(res, rd_idx);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleAddI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::ADD_I32>(pc);
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = frame->getAcc().getValue();
    int32_t rs_i32 = frame->getReg(rs_idx).getValue();
    frame->setAcc(acc_i32 + rs_i32);

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleAddF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::ADD_F>(pc);
    auto rs_idx = instr.getRs();

    auto acc = frame->getAcc().getValue();
    auto rs = frame->getReg(rs_idx).getValue();
    float acc_f = bit::getValue<float>(acc);
    float rs_f = bit::getValue<float>(rs);
    frame->setAcc(bit::castToWritable<float>(acc_f + rs_f));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleSubI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::SUB_I32>(pc);
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = frame->getAcc().getValue();
    int32_t rs_i32 = frame->getReg(rs_idx).getValue();
    frame->setAcc(bit::castToWritable(acc_i32 - rs_i32));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleSubF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::SUB_F>(pc);
    auto rs_idx = instr.getRs();

    auto acc = frame->getAcc().getValue();
    auto rs = frame->getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    frame->setAcc(bit::castToWritable(acc_f - rs_f));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleDivI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::DIV_I32>(pc);
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = frame->getAcc().getValue();
    int32_t rs_i32 = frame->getReg(rs_idx).getValue();
    frame->setAcc(bit::castToWritable(acc_i32 / rs_i32));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleDivF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::DIV_F>(pc);
    auto rs_idx = instr.getRs();

    auto acc = frame->getAcc().getValue();
    auto rs = frame->getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    frame->setAcc(bit::castToWritable(acc_f / rs_f));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMulI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MUL_I32>(pc);
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = frame->getAcc().getValue();
    int32_t rs_i32 = frame->getReg(rs_idx).getValue();
    frame->setAcc(bit::castToWritable(acc_i32 * rs_i32));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMulF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MUL_F>(pc);
    auto rs_idx = instr.getRs();

    auto acc = frame->getAcc().getValue();
    auto rs = frame->getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    frame->setAcc(bit::castToWritable(acc_f * rs_f));

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleIntrinsic(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::INTRINSIC>(pc);

    auto intrinsic_code = static_cast<IntrinsicCode>(instr.getIntrinsicCode());

    auto arg0_idx = instr.getIntrinsicArg0();

    std::cout << "LOG: " << instr.toString() << std::endl;

    switch (intrinsic_code) {
        case IntrinsicCode::PRINT_I32: {
            auto value_raw = frame->getReg(arg0_idx).getValue();
            auto value = bit::getValue<int32_t>(value_raw);

            intrinsics::PrintI(value);
            break;
        }
        case IntrinsicCode::PRINT_F: {
            auto value_raw = frame->getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            intrinsics::PrintF(value);
            break;
        }
        case IntrinsicCode::SCAN_I32: {
            auto res = intrinsics::ScanI();

            frame->setAcc(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::SCAN_F: {
            auto res = intrinsics::ScanF();

            frame->setAcc(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::SIN: {
            auto value_raw = frame->getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::SinF(value);
            frame->setAcc(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::COS: {
            auto value_raw = frame->getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::CosF(value);
            frame->setAcc(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::SQRT: {
            auto value_raw = frame->getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::SqrtF(value);
            frame->setAcc(bit::castToWritable(res));
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

int handleRet(const Byte *pc, [[maybe_unused]] Frame *frame)
{
    Instr<InstrOpcode::RET> instr {pc};

    std::cout << "LOG: " << instr.toString() << std::endl;

    return 0;
}

int handleJump(const Byte *pc, Frame *frame) {
    Instr<InstrOpcode::JUMP> instr {pc};
    int64_t offset = instr.getJumpOffset();

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += offset;
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleJumpGg(const Byte *pc, Frame *frame) {
    Instr<InstrOpcode::JUMP_GG> instr {pc};
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto gg = frame->getAcc().getValue() > frame->getReg(rs_idx).getValue();

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += gg ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleJumpEq(const Byte *pc, Frame *frame) {
    Instr<InstrOpcode::JUMP_EQ> instr {pc};
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto eq = frame->getAcc().getValue() == frame->getReg(rs_idx).getValue();

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += eq ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleJumpLl(const Byte *pc, Frame *frame) {
    Instr<InstrOpcode::JUMP_EQ> instr {pc};
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto ll = frame->getAcc().getValue() < frame->getReg(rs_idx).getValue();

    std::cout << "LOG: " << instr.toString() << std::endl;

    pc += ll ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

}  // namespace

int runImpl(const Byte *pc, Frame *frame)
{
    return dispatch_table[getOpcode(pc)](pc, frame);
}

}  // namespace shrimp::runtime::interpreter
