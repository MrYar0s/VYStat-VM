#include <cassert>
#include <cstdint>
#include <iostream>
#include <array>
#include <sstream>

#include <shrimp/common/logger.hpp>
#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/runtime/interpreter/intrinsics.hpp>
#include <shrimp/runtime/interpreter.hpp>
#include <shrimp/runtime/shrimp_vm.hpp>

#include <shrimp/runtime/interpreter/instr.gen.hpp>
#include <shrimp/runtime/interpreter/dispatch_table.gen.hpp>
#include <shrimp/runtime/register.hpp>

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

        case IntrinsicCode::PRINT_STR:
            out << "PRINT.STR, R" << getIntrinsicArg0();
            break;

        case IntrinsicCode::CONCAT:
            out << "CONCAT, R" << getIntrinsicArg0() << ", R" << getIntrinsicArg1();
            break;

        case IntrinsicCode::SUBSTR:
            out << "SUBSTR, R" << getIntrinsicArg0() << ", R" << getIntrinsicArg1();
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
    auto arg1_idx = instr.getIntrinsicArg1();

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
        case IntrinsicCode::PRINT_STR: {
            auto str_id = frame.getReg(arg0_idx).getValue();
            const std::string &str = vm->resolveString(str_id);

            intrinsics::PrintStr(str);
            break;
        }
        case IntrinsicCode::CONCAT: {
            auto str0_id = frame.getReg(arg0_idx).getValue();
            auto str1_id = frame.getReg(arg1_idx).getValue();
            const std::string &str0 = vm->resolveString(str0_id);
            const std::string &str1 = vm->resolveString(str1_id);

            const std::string &str = intrinsics::Concat(str0, str1);
            uint64_t str_id = vm->addStringToAccessor(str);

            vm->acc().setValue(bit::castToWritable(str_id));

            break;
        }
        case IntrinsicCode::SUBSTR: {
            auto pos = frame.getReg(arg0_idx).getValue();
            auto len = frame.getReg(arg1_idx).getValue();
            auto str_id = vm->acc().getValue();
            const std::string &str = vm->resolveString(str_id);

            const std::string &substr = intrinsics::Substr(str, pos, len);
            uint64_t substr_id = vm->addStringToAccessor(substr);

            vm->acc().setValue(bit::castToWritable(substr_id));

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

int handleCall0arg(ShrimpVM *vm)
{
    Instr<InstrOpcode::CALL_0ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->stack().push(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});

    auto &frame = vm->currFrame();

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleCall1arg(ShrimpVM *vm)
{
    Instr<InstrOpcode::CALL_1ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    vm->stack().push(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = prev_frame.getReg(func_0arg_idx).getValue();
    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255);

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleCall2arg(ShrimpVM *vm)
{
    Instr<InstrOpcode::CALL_2ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();
    auto func_1arg_idx = instr.getFuncArg1();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    vm->stack().push(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = prev_frame.getReg(func_0arg_idx).getValue();
    auto func_1arg = prev_frame.getReg(func_1arg_idx).getValue();
    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255);
    frame.setReg(func_1arg, 254);

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleCall3arg(ShrimpVM *vm)
{
    Instr<InstrOpcode::CALL_3ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();
    auto func_1arg_idx = instr.getFuncArg1();
    auto func_2arg_idx = instr.getFuncArg2();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    vm->stack().push(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = prev_frame.getReg(func_0arg_idx).getValue();
    auto func_1arg = prev_frame.getReg(func_1arg_idx).getValue();
    auto func_2arg = prev_frame.getReg(func_2arg_idx).getValue();
    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255);
    frame.setReg(func_1arg, 254);
    frame.setReg(func_2arg, 253);

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleCall4arg(ShrimpVM *vm)
{
    Instr<InstrOpcode::CALL_4ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();
    auto func_1arg_idx = instr.getFuncArg1();
    auto func_2arg_idx = instr.getFuncArg2();
    auto func_3arg_idx = instr.getFuncArg3();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    vm->stack().push(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = prev_frame.getReg(func_0arg_idx).getValue();
    auto func_1arg = prev_frame.getReg(func_1arg_idx).getValue();
    auto func_2arg = prev_frame.getReg(func_2arg_idx).getValue();
    auto func_3arg = prev_frame.getReg(func_3arg_idx).getValue();
    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255);
    frame.setReg(func_1arg, 254);
    frame.setReg(func_2arg, 253);
    frame.setReg(func_3arg, 252);

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleRet(ShrimpVM *vm)
{
    Instr<InstrOpcode::RET> instr {vm->pc()};
    auto &frame = vm->currFrame();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto *ret_pc = frame.getRetPc();
    if (ret_pc != nullptr) {
        vm->pc() = ret_pc;
        vm->stack().pop();
        return dispatch_table[getOpcode(vm->pc())](vm);
    }

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

    auto gg = bit::getValue<int32_t>(vm->acc().getValue()) > bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

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

    auto eq = bit::getValue<int32_t>(vm->acc().getValue()) == bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += eq ? offset : instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleJumpLl(ShrimpVM *vm)
{
    Instr<InstrOpcode::JUMP_LL> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto ll = bit::getValue<int32_t>(vm->acc().getValue()) < bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

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

int handleLdaStr(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::LDA_STR>(vm->pc());

    auto str_id = instr.getStrId();
    vm->acc().setValue(bit::castToWritable(str_id));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleArrNewI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ARR_NEW_I32>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto size = frame.getReg(rs_idx).getValue();

    int32_t *ptr = std::bit_cast<int32_t *>(vm->getAllocator().allocate(size));

    frame.setReg(bit::castToWritable(ptr), rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleCmpEqI32(ShrimpVM *vm)
{
    Instr<InstrOpcode::CMP_EQ_I32> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto eq = bit::getValue<int32_t>(vm->acc().getValue()) == bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(static_cast<uint32_t>(eq)));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleArrNewF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ARR_NEW_F>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto size = frame.getReg(rs_idx).getValue();

    float *ptr = std::bit_cast<float *>(vm->getAllocator().allocate(size));

    frame.setReg(bit::castToWritable(ptr), rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}
int handleCmpGgI32(ShrimpVM *vm)
{
    Instr<InstrOpcode::CMP_GG_I32> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto gg = bit::getValue<int32_t>(vm->acc().getValue()) > bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(static_cast<uint32_t>(gg)));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleArrLdaI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ARR_LDA_I32>(vm->pc());

    auto &frame = vm->currFrame();

    auto rs1_idx = instr.getRs1();
    auto rs2_idx = instr.getRs2();

    auto pos = frame.getReg(rs2_idx).getValue();
    auto ptr = std::bit_cast<int32_t *>(frame.getReg(rs1_idx).getValue());

    vm->acc().setValue(bit::castToWritable(ptr[pos]));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleArrLdaF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ARR_LDA_F>(vm->pc());

    auto &frame = vm->currFrame();

    auto rs1_idx = instr.getRs1();
    auto rs2_idx = instr.getRs2();

    auto pos = frame.getReg(rs2_idx).getValue();
    auto ptr = std::bit_cast<float *>(frame.getReg(rs1_idx).getValue());

    vm->acc().setValue(bit::castToWritable(ptr[pos]));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleArrStaI32(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ARR_STA_I32>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto acc_val = vm->acc().getValue();

    auto pos = frame.getReg(rs_idx).getValue();
    auto ptr = std::bit_cast<int32_t *>(frame.getReg(rd_idx).getValue());

    ptr[pos] = acc_val;

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleArrStaF(ShrimpVM *vm)
{
    auto instr = Instr<InstrOpcode::ARR_STA_F>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto acc_val = vm->acc().getValue();

    auto pos = frame.getReg(rs_idx).getValue();
    auto ptr = std::bit_cast<float *>(frame.getReg(rd_idx).getValue());

    ptr[pos] = acc_val;

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleCmpLlI32(ShrimpVM *vm)
{
    Instr<InstrOpcode::CMP_LL_I32> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto ll = bit::getValue<int32_t>(vm->acc().getValue()) < bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(static_cast<uint32_t>(ll)));

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleObjNew(ShrimpVM *vm)
{
    Instr<InstrOpcode::OBJ_NEW> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();

    auto class_size = vm->getClasses().find(instr.getClassId())->second.size;
    auto new_obj_ptr = vm->getAllocator().allocate(class_size);

    frame.setReg(bit::castToWritable(new_obj_ptr), rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleLdfield(ShrimpVM *vm)
{
    Instr<InstrOpcode::LDFIELD> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();
    auto field = vm->resolveField(instr.getClassId(), instr.getFieldId());
    uint64_t ld_tmp = 0;

    auto class_ptr = bit::getValue<Byte *>(frame.getReg(rs_idx).getValue());

    if constexpr (std::endian::native == std::endian::big) {
        std::memcpy(&ld_tmp + sizeof(ld_tmp) - field.size, class_ptr + field.offset, field.size);
    } else {
        std::memcpy(&ld_tmp, class_ptr + field.offset, field.size);
    }

    frame.setReg(ld_tmp, rd_idx);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    return dispatch_table[getOpcode(vm->pc())](vm);
}

int handleStfield(ShrimpVM *vm)
{
    Instr<InstrOpcode::STFIELD> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto field = vm->resolveField(instr.getClassId(), instr.getFieldId());

    auto class_ptr = bit::getValue<Byte *>(frame.getReg(rd_idx).getValue());
    uint64_t field_val = frame.getReg(rs_idx).getValue();

    if constexpr (std::endian::native == std::endian::big) {
        std::memcpy(class_ptr + field.offset, &field_val + sizeof(field_val) - field.size, field.size);
    } else {
        std::memcpy(class_ptr + field.offset, &field_val, field.size);
    }

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
