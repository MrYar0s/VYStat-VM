#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <array>
#include <optional>
#include <sstream>

#include <shrimp/common/logger.hpp>
#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/runtime/interpreter/intrinsics.hpp>
#include <shrimp/runtime/interpreter.hpp>
#include <shrimp/runtime/shrimp_vm.hpp>

#include <shrimp/runtime/interpreter/instr.gen.hpp>
#include <shrimp/runtime/register.hpp>

#include <shrimp/runtime/coretypes/string.hpp>
#include <shrimp/runtime/coretypes/array.hpp>
#include <shrimp/runtime/coretypes/class.hpp>

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

static constexpr Byte OPCODE_MASK = 0xff;

Byte getOpcode(const Byte *pc)
{
    return *pc & OPCODE_MASK;
}

int runImpl(ShrimpVM *vm)
{
#include <shrimp/runtime/interpreter/dispatch_table.gen.inl>

    goto *dispatch_table[getOpcode(vm->pc())];

handleInvalidOpcode : {
    return -1;
}
handleNop : {
    Instr<InstrOpcode::NOP> instr {vm->pc()};

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleMov : {
    auto instr = Instr<InstrOpcode::MOV>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto rs_reg = frame.getReg(rs_idx);
    auto res = rs_reg.getValue();
    frame.setReg(res, rd_idx, rs_reg.getRefMark());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleMovImmI32 : {
    auto instr = Instr<InstrOpcode::MOV_IMM_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto imm_i32 = bit::getValue<int32_t>(instr.getImmI32());

    frame.setReg(imm_i32, rd_idx, false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleMovImmF : {
    auto instr = Instr<InstrOpcode::MOV_IMM_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto imm_f = bit::getValue<float>(instr.getImmF());

    frame.setReg(bit::castToWritable(imm_f), rd_idx, false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleLda : {
    auto instr = Instr<InstrOpcode::LDA>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto res_reg = frame.getReg(rs_idx);
    auto res = res_reg.getValue();
    vm->acc().setValue(res, res_reg.getRefMark());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleLdaImmI32 : {
    auto instr = Instr<InstrOpcode::LDA_IMM_I32>(vm->pc());
    auto imm_i32 = bit::getValue<int32_t>(instr.getImmI32());

    vm->acc().setValue(imm_i32, false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleLdaImmF : {
    auto instr = Instr<InstrOpcode::LDA_IMM_F>(vm->pc());
    auto imm_f = bit::getValue<float>(instr.getImmF());

    vm->acc().setValue(bit::castToWritable(imm_f), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleSta : {
    auto instr = Instr<InstrOpcode::STA>(vm->pc());
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();

    auto acc = vm->acc();
    auto res = acc.getValue();
    frame.setReg(res, rd_idx, acc.getRefMark());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleAddI32 : {
    auto instr = Instr<InstrOpcode::ADD_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    vm->acc().setValue(acc_i32 + rs_i32, false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleAddF : {
    auto instr = Instr<InstrOpcode::ADD_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    float acc_f = bit::getValue<float>(acc);
    float rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable<float>(acc_f + rs_f), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleSubI32 : {
    auto instr = Instr<InstrOpcode::SUB_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    vm->acc().setValue(bit::castToWritable(acc_i32 - rs_i32), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleSubF : {
    auto instr = Instr<InstrOpcode::SUB_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable(acc_f - rs_f), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleMod : {
    auto instr = Instr<InstrOpcode::MOD>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    auto res = bit::signExtend<DWord, 31>(acc_i32 % rs_i32);
    vm->acc().setValue(bit::castToWritable(res), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleDivI32 : {
    auto instr = Instr<InstrOpcode::DIV_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();
    auto res = bit::signExtend<DWord, 31>(acc_i32 / rs_i32);
    vm->acc().setValue(bit::castToWritable(res), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleDivF : {
    auto instr = Instr<InstrOpcode::DIV_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable(acc_f / rs_f), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleMulI32 : {
    auto instr = Instr<InstrOpcode::MUL_I32>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    int32_t acc_i32 = vm->acc().getValue();
    int32_t rs_i32 = frame.getReg(rs_idx).getValue();

    auto res = bit::signExtend<DWord, 31>(acc_i32 * rs_i32);
    vm->acc().setValue(bit::castToWritable(res), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleMulF : {
    auto instr = Instr<InstrOpcode::MUL_F>(vm->pc());
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto acc = vm->acc().getValue();
    auto rs = frame.getReg(rs_idx).getValue();
    auto acc_f = bit::getValue<float>(acc);
    auto rs_f = bit::getValue<float>(rs);
    vm->acc().setValue(bit::castToWritable(acc_f * rs_f), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleIntrinsic : {
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
            auto ptr = frame.getReg(arg0_idx).getValue();
            auto strObj = reinterpret_cast<String *>(bit::getValue<int32_t *>(ptr));
            const std::string &str = strObj->getData();

            intrinsics::PrintStr(str);
            break;
        }
        case IntrinsicCode::CONCAT: {
            vm->triggerGCIfNeed();
            auto ptr0 = frame.getReg(arg0_idx).getValue();
            auto strObj0 = reinterpret_cast<String *>(bit::getValue<int32_t *>(ptr0));
            auto ptr1 = frame.getReg(arg1_idx).getValue();
            auto strObj1 = reinterpret_cast<String *>(bit::getValue<int32_t *>(ptr1));

            auto strObj = String::ConcatStrings(strObj0, strObj1, vm);

            auto ptr = std::bit_cast<int32_t *>(strObj);

            vm->acc().setValue(bit::castToWritable(ptr), true);
            break;
        }
        case IntrinsicCode::SUBSTR: {
            vm->triggerGCIfNeed();
            auto pos = frame.getReg(arg0_idx).getValue();
            auto len = frame.getReg(arg1_idx).getValue();
            auto ptr = vm->acc().getValue();
            auto strObj = reinterpret_cast<String *>(bit::getValue<int32_t *>(ptr));

            auto newStrObj = String::SubStr(strObj, pos, len, vm);

            auto newPtr = std::bit_cast<int32_t *>(newStrObj);
            vm->acc().setValue(bit::castToWritable(newPtr), true);

            break;
        }
        case IntrinsicCode::SCAN_I32: {
            auto res = intrinsics::ScanI();

            vm->acc().setValue(bit::castToWritable(res), false);
            break;
        }
        case IntrinsicCode::SCAN_F: {
            auto res = intrinsics::ScanF();

            vm->acc().setValue(bit::castToWritable(res), false);
            break;
        }
        case IntrinsicCode::SIN: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::SinF(value);
            vm->acc().setValue(bit::castToWritable(res), false);
            break;
        }
        case IntrinsicCode::COS: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::CosF(value);
            vm->acc().setValue(bit::castToWritable(res), false);
            break;
        }
        case IntrinsicCode::SQRT: {
            auto value_raw = frame.getReg(arg0_idx).getValue();
            auto value = bit::getValue<float>(value_raw);

            auto res = intrinsics::SqrtF(value);
            vm->acc().setValue(bit::castToWritable(res), false);
            break;
        }
        default: {
            LOG_INFO("Unsupported intrinsic", vm->getLogLevel());
            std::abort();
        }
    }
    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCall0arg : {
    Instr<InstrOpcode::CALL_0ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->stack().push_back(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});

    auto &frame = vm->currFrame();

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCall1arg : {
    Instr<InstrOpcode::CALL_1ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    auto reg0 = prev_frame.getReg(func_0arg_idx);
    vm->stack().push_back(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = reg0.getValue();
    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255, reg0.getRefMark());

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCall2arg : {
    Instr<InstrOpcode::CALL_2ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();
    auto func_1arg_idx = instr.getFuncArg1();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    auto reg0 = prev_frame.getReg(func_0arg_idx);
    auto reg1 = prev_frame.getReg(func_1arg_idx);
    vm->stack().push_back(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = reg0.getValue();
    auto func_1arg = reg1.getValue();
    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255, reg0.getRefMark());
    frame.setReg(func_1arg, 254, reg1.getRefMark());

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCall3arg : {
    Instr<InstrOpcode::CALL_3ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();
    auto func_1arg_idx = instr.getFuncArg1();
    auto func_2arg_idx = instr.getFuncArg2();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    auto reg0 = prev_frame.getReg(func_0arg_idx);
    auto reg1 = prev_frame.getReg(func_1arg_idx);
    auto reg2 = prev_frame.getReg(func_2arg_idx);
    vm->stack().push_back(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = reg0.getValue();
    auto func_1arg = reg1.getValue();
    auto func_2arg = reg2.getValue();

    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255, reg0.getRefMark());
    frame.setReg(func_1arg, 254, reg1.getRefMark());
    frame.setReg(func_2arg, 253, reg2.getRefMark());

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCall4arg : {
    Instr<InstrOpcode::CALL_4ARG> instr {vm->pc()};

    auto func_id = instr.getFuncId();
    auto func_0arg_idx = instr.getFuncArg0();
    auto func_1arg_idx = instr.getFuncArg1();
    auto func_2arg_idx = instr.getFuncArg2();
    auto func_3arg_idx = instr.getFuncArg3();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto &prev_frame = vm->currFrame();
    auto reg0 = prev_frame.getReg(func_0arg_idx);
    auto reg1 = prev_frame.getReg(func_1arg_idx);
    auto reg2 = prev_frame.getReg(func_2arg_idx);
    auto reg3 = prev_frame.getReg(func_3arg_idx);
    vm->stack().push_back(Frame {std::make_shared<RuntimeFunc>(vm->resolveFunc(func_id))});
    auto &frame = vm->currFrame();

    auto func_0arg = reg0.getValue();
    auto func_1arg = reg1.getValue();
    auto func_2arg = reg2.getValue();
    auto func_3arg = reg3.getValue();

    // TODO(VasiliyMatr): replace magic number to function
    frame.setReg(func_0arg, 255, reg0.getRefMark());
    frame.setReg(func_1arg, 254, reg1.getRefMark());
    frame.setReg(func_2arg, 253, reg2.getRefMark());
    frame.setReg(func_3arg, 252, reg3.getRefMark());

    ByteOffset offset = frame.getOffsetToFunc();
    frame.setRetPc(vm->pc() + instr.getByteSize());
    vm->pc() = vm->getPcFromStart(offset);

    goto *dispatch_table[getOpcode(vm->pc())];
}
handleRet : {
    Instr<InstrOpcode::RET> instr {vm->pc()};
    auto &frame = vm->currFrame();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    auto *ret_pc = frame.getRetPc();
    if (ret_pc != nullptr) {
        vm->pc() = ret_pc;
        vm->stack().pop_back();
        goto *dispatch_table[getOpcode(vm->pc())];
    }

    return 0;
}
handleJump : {
    Instr<InstrOpcode::JUMP> instr {vm->pc()};
    int64_t offset = instr.getJumpOffset();

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += offset;
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleJumpGg : {
    Instr<InstrOpcode::JUMP_GG> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto gg = bit::getValue<int32_t>(vm->acc().getValue()) > bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += gg ? offset : instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleJumpNotEq : {
    Instr<InstrOpcode::JUMP_EQ> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto eq = bit::getValue<int32_t>(vm->acc().getValue()) != bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += eq ? offset : instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleJumpEq : {
    Instr<InstrOpcode::JUMP_EQ> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto eq = bit::getValue<int32_t>(vm->acc().getValue()) == bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += eq ? offset : instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleJumpLl : {
    Instr<InstrOpcode::JUMP_LL> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();
    auto offset = instr.getJumpOffset();

    auto ll = bit::getValue<int32_t>(vm->acc().getValue()) < bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += ll ? offset : instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleI32tof : {
    auto instr = Instr<InstrOpcode::I32TOF>(vm->pc());

    int32_t acc_i32 = vm->acc().getValue();
    float acc_f = acc_i32;
    vm->acc().setValue(bit::castToWritable(acc_f), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleFtoi32 : {
    auto instr = Instr<InstrOpcode::FTOI32>(vm->pc());

    auto acc = vm->acc().getValue();
    auto acc_f = bit::getValue<float>(acc);
    int32_t acc_i = acc_f;
    vm->acc().setValue(bit::castToWritable(acc_i), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleLdaStr : {
    vm->triggerGCIfNeed();
    auto instr = Instr<InstrOpcode::LDA_STR>(vm->pc());

    auto str_id = instr.getStrId();
    const auto &str = vm->resolveString(str_id);
    auto strObj = String::AllocateString(str.size(), str.data(), vm);

    auto ptr = std::bit_cast<int32_t *>(strObj);

    vm->acc().setValue(bit::castToWritable(ptr), true);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrLength : {
    auto instr = Instr<InstrOpcode::ARR_LENGTH>(vm->pc());

    auto &frame = vm->currFrame();

    auto rs_idx = instr.getRs();

    auto arrObj = std::bit_cast<Array *>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(arrObj->getSize()), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrNewI32 : {
    vm->triggerGCIfNeed();
    auto instr = Instr<InstrOpcode::ARR_NEW_I32>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto size = frame.getReg(rs_idx).getValue();

    auto arrObj = Array::AllocateArray(0, size, vm);

    int32_t *ptr = std::bit_cast<int32_t *>(arrObj);

    frame.setReg(bit::castToWritable(ptr), rd_idx, true);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCmpEqI32 : {
    Instr<InstrOpcode::CMP_EQ_I32> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto eq = bit::getValue<int32_t>(vm->acc().getValue()) == bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(static_cast<uint32_t>(eq)), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrNewF : {
    vm->triggerGCIfNeed();
    auto instr = Instr<InstrOpcode::ARR_NEW_F>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto size = frame.getReg(rs_idx).getValue();

    auto arrObj = Array::AllocateArray(0, size, vm);

    int32_t *ptr = std::bit_cast<int32_t *>(arrObj);

    frame.setReg(bit::castToWritable(ptr), rd_idx, true);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCmpGgI32 : {
    Instr<InstrOpcode::CMP_GG_I32> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto gg = bit::getValue<int32_t>(vm->acc().getValue()) > bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(static_cast<uint32_t>(gg)), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrNewRef : {
    vm->triggerGCIfNeed();
    auto instr = Instr<InstrOpcode::ARR_NEW_REF>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();
    auto class_id = instr.getClassId();

    auto size = frame.getReg(rs_idx).getValue();

    const auto &klass = vm->getClasses()[class_id];

    auto arrObj = Array::AllocateArrayRef(klass, size, vm);

    int32_t *ptr = std::bit_cast<int32_t *>(arrObj);

    frame.setReg(bit::castToWritable(ptr), rd_idx, true);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrLdaI32 : {
    auto instr = Instr<InstrOpcode::ARR_LDA_I32>(vm->pc());

    auto &frame = vm->currFrame();

    auto rs1_idx = instr.getRs1();
    auto rs2_idx = instr.getRs2();

    auto pos = frame.getReg(rs2_idx).getValue();
    auto ptr = std::bit_cast<Array *>(frame.getReg(rs1_idx).getValue());

    vm->acc().setValue(bit::castToWritable(ptr->getElem(pos)), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrLdaF : {
    auto instr = Instr<InstrOpcode::ARR_LDA_F>(vm->pc());

    auto &frame = vm->currFrame();

    auto rs1_idx = instr.getRs1();
    auto rs2_idx = instr.getRs2();

    auto pos = frame.getReg(rs2_idx).getValue();

    auto ptr = std::bit_cast<Array *>(frame.getReg(rs1_idx).getValue());

    vm->acc().setValue(bit::castToWritable(ptr->getElem(pos)), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrLdaRef : {
    auto instr = Instr<InstrOpcode::ARR_LDA_REF>(vm->pc());

    auto &frame = vm->currFrame();

    auto rs1_idx = instr.getRs1();
    auto rs2_idx = instr.getRs2();

    auto pos = frame.getReg(rs2_idx).getValue();

    auto ptr = std::bit_cast<Array *>(frame.getReg(rs1_idx).getValue());

    auto runtimeClassFromArr = reinterpret_cast<RuntimeArray *>(ptr->getClassWord());
    if (runtimeClassFromArr != nullptr) {
        LOG_INFO("Name of class from array : " + runtimeClassFromArr->klass->name, vm->getLogLevel());
    } else {
        return -1;
    }

    vm->acc().setValue(bit::castToWritable(ptr->getElem(pos)), true);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrStaI32 : {
    auto instr = Instr<InstrOpcode::ARR_STA_I32>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto acc_val = vm->acc().getValue();

    auto pos = frame.getReg(rs_idx).getValue();
    auto ptr = std::bit_cast<Array *>(frame.getReg(rd_idx).getValue());

    ptr->setElem(acc_val, pos);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrStaF : {
    auto instr = Instr<InstrOpcode::ARR_STA_F>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto acc_val = vm->acc().getValue();

    auto pos = frame.getReg(rs_idx).getValue();
    auto ptr = std::bit_cast<Array *>(frame.getReg(rd_idx).getValue());

    ptr->setElem(acc_val, pos);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleArrStaRef : {
    auto instr = Instr<InstrOpcode::ARR_STA_REF>(vm->pc());

    auto &frame = vm->currFrame();

    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    auto acc_val = vm->acc().getValue();

    auto pos = frame.getReg(rs_idx).getValue();
    auto ptr = std::bit_cast<Array *>(frame.getReg(rd_idx).getValue());

    auto accAsClass = reinterpret_cast<Class *>(acc_val);

    auto runtimeClassFromArr = reinterpret_cast<RuntimeClass *>(ptr->getClassWord());
    auto runtimeClassFromAcc = reinterpret_cast<RuntimeClass *>(accAsClass->getClassWord());
    if (runtimeClassFromArr != nullptr && runtimeClassFromAcc != nullptr) {
        if (runtimeClassFromArr->name != runtimeClassFromAcc->name) {
            LOG_INFO("Name of class from array : " + runtimeClassFromArr->name, vm->getLogLevel());
            LOG_INFO("Name of class from accumulator : " + runtimeClassFromAcc->name, vm->getLogLevel());
        }
    } else {
        return -1;
    }

    LOG_INFO("pos to save : " << pos, vm->getLogLevel());

    ptr->setElem(acc_val, pos);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleCmpLlI32 : {
    Instr<InstrOpcode::CMP_LL_I32> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rs_idx = instr.getRs();

    auto ll = bit::getValue<int32_t>(vm->acc().getValue()) < bit::getValue<int32_t>(frame.getReg(rs_idx).getValue());

    vm->acc().setValue(bit::castToWritable(static_cast<uint32_t>(ll)), false);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleObjNew : {
    vm->triggerGCIfNeed();
    Instr<InstrOpcode::OBJ_NEW> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();

    auto class_id = instr.getClassId();

    const auto &klass = vm->getClasses()[class_id];

    auto class_obj = Class::AllocateClassRef(reinterpret_cast<uint64_t>(&klass), klass.size, vm);
    auto ptr = reinterpret_cast<int32_t *>(class_obj);

    frame.setReg(bit::castToWritable(ptr), rd_idx, true);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleLdfield : {
    Instr<InstrOpcode::LDFIELD> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();
    const auto &field = vm->resolveField(instr.getClassId(), instr.getFieldId());

    const auto &classIdFromInstr = vm->getClasses()[instr.getClassId()];
    LOG_INFO("Name of class from instr : " << classIdFromInstr.name, vm->getLogLevel());

    auto class_ptr = std::bit_cast<Class *>(frame.getReg(rs_idx).getValue());
    LOG_INFO("Class ptr from reg : " << class_ptr, vm->getLogLevel());

    LOG_INFO("Name of class from ptr : " << reinterpret_cast<RuntimeClass *>(class_ptr->getClassWord())->name,
             vm->getLogLevel());

    uint64_t ld_tmp = class_ptr->getField(field);

    frame.setReg(ld_tmp, rd_idx, field.is_ref);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
handleStfield : {
    Instr<InstrOpcode::STFIELD> instr {vm->pc()};
    auto &frame = vm->currFrame();
    auto rd_idx = instr.getRd();
    auto rs_idx = instr.getRs();

    const auto &field = vm->resolveField(instr.getClassId(), instr.getFieldId());
    uint64_t field_val = frame.getReg(rs_idx).getValue();

    auto class_ptr = std::bit_cast<Class *>(frame.getReg(rd_idx).getValue());

    class_ptr->setField(field, field_val);

    LOG_INFO(instr.toString(), vm->getLogLevel());

    vm->pc() += instr.getByteSize();
    goto *dispatch_table[getOpcode(vm->pc())];
}
}

}  // namespace shrimp::runtime::interpreter