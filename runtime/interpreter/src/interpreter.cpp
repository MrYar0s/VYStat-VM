#include <cassert>
#include <iostream>
#include <array>
#include <sstream>

#include <shrimp/common/logger.hpp>
#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/runtime/interpreter/intrinsics.hpp>
#include <shrimp/runtime/interpreter.hpp>

#include <shrimp/runtime/interpreter/instr.gen.hpp>
#include <shrimp/runtime/interpreter/dispatch_table.gen.hpp>

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

int handleNop(ShrimpVM *vm)
{
    Instr<InstrOpcode::NOP> instr {vm->pc()};

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleMov(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::MOV>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto res = frame.getReg(rs_idx).getValue();
    frame.setReg(res, rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleMovImmI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto imm_i32 = bit::getValue<int32_t>(instr.getImmI32());

    frame.setReg(imm_i32, rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleMovImmF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto imm_f = bit::getValue<float>(instr.getImmF());

    frame.setReg(bit::castToWritable(imm_f), rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleLda(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::LDA>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto res = frame.getReg(rs_idx).getValue();
    vm->acc().setValue(res);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleLdaImmI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_I32>(vm->pc());
    auto imm_i32 = bit::getValue<int32_t>(instr.getImmI32());

    vm->acc().setValue(imm_i32);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleLdaImmF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_F>(vm->pc());
    auto imm_f = bit::getValue<float>(instr.getImmF());

    vm->acc().setValue(bit::castToWritable(imm_f));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleSta(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::STA>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();

    auto res = vm->acc().getValue();
    frame.setReg(res, rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleAddI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ADD_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    vm->acc().setValue(acc_i32 + rs_i32);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleAddF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ADD_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    float acc_f = bit::getValue<float>(acc);
    float rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable<float>(acc_f + rs_f));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleSubI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::SUB_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    vm->acc().setValue(bit::castToWritable(acc_i32 - rs_i32));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleSubF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::SUB_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable(acc_f - rs_f));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleDivI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::DIV_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    auto res = bit::signExtend<DWord, 31>(acc_i32 / rs_i32);
    vm->acc().setValue(bit::castToWritable(res));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleDivF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::DIV_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable(acc_f / rs_f));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleMulI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::MUL_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();

    auto res = bit::signExtend<DWord, 31>(acc_i32 * rs_i32);
    vm->acc().setValue(bit::castToWritable(res));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleMulF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::MUL_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable(acc_f * rs_f));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleIntrinsic(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::INTRINSIC>(vm->pc());
    auto &frame = vm->currFrame();

    auto intrinsic_code = static_cast<IntrinsicCode>(instr.getIntrinsicCode());

    auto arg0_idx = instr.getIntrinsicArg0();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    switch (intrinsic_code) {
        case IntrinsicCode::PRINT_I32: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<int32_t>(value_raw);

            intrinsics::PrintI(value);
            break;
        }
        case IntrinsicCode::PRINT_F: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            intrinsics::PrintF(value);
            break;
        }
        case IntrinsicCode::SCAN_I32: {
            auto res = intrinsics::ScanI();

            vm->acc().setValue(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::SCAN_F: {
            auto res = intrinsics::ScanF();

            vm->acc().setValue(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::SIN: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::SinF(value);
            vm->acc().setValue(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::COS: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::CosF(value);
            vm->acc().setValue(bit::castToWritable(res));
            break;
        }
        case IntrinsicCode::SQRT: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::SqrtF(value);
            vm->acc().setValue(bit::castToWritable(res));
            break;
        }
        default: {
            LOG_ERROR("Unsupported intrinsic", vm->getLogLevel());
            std::abort();
        }
    }
    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleRet(ShrimpVM *vm)
{
    Instr<InstrOpcode::RET> instr {vm->pc()};

    LOG_INFO(instr.toString(), vm->getLogLevel());

    return 0;
}

int handleJump(ShrimpVM *vm)
{
    Instr<InstrOpcode::JUMP> instr {vm->pc()};
    int64_t offset = instr.getJumpOffset();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += offset;
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleJumpGg(ShrimpVM *vm)
{
    Instr<InstrOpcode::JUMP_GG> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto gg = vm->acc().getValue() > frame.getReg(rs_idx).getValue();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += gg ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleJumpEq(ShrimpVM *vm)
{
    Instr<InstrOpcode::JUMP_EQ> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto eq = vm->acc().getValue() == frame.getReg(rs_idx).getValue();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += eq ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleJumpLl(ShrimpVM *vm)
{
    Instr<InstrOpcode::JUMP_EQ> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto ll = vm->acc().getValue() < frame.getReg(rs_idx).getValue();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += ll ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleI32tof(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::I32TOF>(vm->pc());

    int32_t acc_i32 = vm->acc().getValue();
    float acc_f = acc_i32;
    vm->acc().setValue(bit::castToWritable(acc_f));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleFtoi32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::FTOI32>(vm->pc());

    auto acc = vm->acc().getValue();
    auto acc_f = bit::getValue<float>(acc);
    int32_t acc_i = acc_f;
    vm->acc().setValue(bit::castToWritable(acc_i));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

}  // namespace

int runImpl(ShrimpVM *vm)
{
    return dispatch_table[getOpcode(vm->pc())](vm);
}

}  // namespace shrimp::runtime::interpreter
