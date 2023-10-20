#include <iostream>
#include <array>

#include <shrimp/common/bitops.hpp>
#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/runtime/shrimp_vm/instr.gen.hpp>
#include <shrimp/runtime/shrimp_vm/intrinsics.hpp>
#include <shrimp/runtime/shrimp_vm/interpreter.hpp>

namespace shrimp::runtime::interpreter {

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

uint64_t getOpcode(const Byte *pc)
{
    return *pc & OPCODE_MASK;
}

int handleNop(const Byte *pc, Frame *frame)
{
    std::cout << "LOG_INFO: "
              << "nop" << std::endl;

    pc += Instr<InstrOpcode::NOP>::getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMov(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV>(pc);
    auto dst_reg_num = instr.getRd();
    auto src_reg_num = instr.getRs();

    auto src_reg = frame->getReg(src_reg_num);
    frame->setReg(src_reg.getValue(), dst_reg_num);

    std::cout << "LOG_INFO: "
              << "mov "
              << "r" << (size_t)dst_reg_num << " r" << (size_t)src_reg_num << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMovImmF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_F>(pc);
    auto dst_reg_num = instr.getRd();
    auto val = bit::getValue<float>(instr.getImm_f());

    auto res = bit::castToWritable<float>(val);
    frame->setReg(res, dst_reg_num);

    std::cout << "LOG_INFO: "
              << "mov.imm.f "
              << "r" << (size_t)dst_reg_num << " " << val << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleMovImmI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::MOV_IMM_I32>(pc);
    auto dst_reg_num = instr.getRd();
    auto val = bit::getValue<int>(instr.getImm_i32());

    auto res = bit::castToWritable<int>(val);
    frame->setReg(res, dst_reg_num);

    std::cout << "LOG_INFO: "
              << "mov.imm.i32 "
              << "r" << (size_t)dst_reg_num << " " << val << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLda(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA>(pc);
    auto src_reg_num = instr.getRs();

    auto src_reg = frame->getReg(src_reg_num);
    frame->setAcc(src_reg.getValue());

    std::cout << "LOG_INFO: "
              << "lda "
              << "r" << (size_t)src_reg_num << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLdaImmF(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_F>(pc);
    auto val = bit::getValue<float>(instr.getImm_f());

    auto res = bit::castToWritable<float>(val);
    frame->setAcc(res);

    std::cout << "LOG_INFO: "
              << "lda.imm.f " << val << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleLdaImmI32(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::LDA_IMM_I32>(pc);
    auto val = bit::getValue<int>(instr.getImm_i32());

    auto res = bit::castToWritable<int>(val);
    frame->setAcc(res);

    std::cout << "LOG_INFO: "
              << "lda.imm.i32 " << val << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleSta(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::STA>(pc);
    auto dst_reg_num = instr.getRd();

    auto acc = frame->getAcc();
    frame->setReg(acc.getValue(), dst_reg_num);

    std::cout << "LOG_INFO: "
              << "sta "
              << "r" << (size_t)dst_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "add.i32 "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "add.f "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "sub.i32 "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "sub.f "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "div.i32 "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "div.f "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "mul.i32 "
              << "r" << (size_t)src_reg_num << std::endl;

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

    std::cout << "LOG_INFO: "
              << "mul.f "
              << "r" << (size_t)src_reg_num << std::endl;

    pc += instr.getByteSize();
    return dispatch_table[getOpcode(pc)](pc, frame);
}

int handleIntrinsic(const Byte *pc, Frame *frame)
{
    auto instr = Instr<InstrOpcode::INTRINSIC>(pc);

    auto intrinsic_type = static_cast<IntrinsicCode>(instr.getIntrinsic_code());

    [[maybe_unused]] auto first_reg_num = instr.getIntrinsic_arg_0();
    [[maybe_unused]] auto second_reg_num = instr.getIntrinsic_arg_1();

    switch (intrinsic_type) {
        case IntrinsicCode::PRINT_I32: {
            std::cout << "LOG_INFO: "
                      << "intrinsic print.i32, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto reg = frame->getReg(first_reg_num);
            auto first_reg = bit::getValue<int>(reg.getValue());
            intrinsics::PrintI(first_reg);
            break;
        }
        case IntrinsicCode::PRINT_F: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic print.f, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = bit::getValue<float>(reg.getValue());
            intrinsics::PrintF(first_reg);
            break;
        }
        case IntrinsicCode::SCAN_I32: {
            std::cout << "LOG_INFO: "
                      << "intrinsic scan.i32" << std::endl;
            int res = intrinsics::ScanI();
            uint64_t res_u64 = bit::castToWritable<int>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SCAN_F: {
            std::cout << "LOG_INFO: "
                      << "intrinsic scan.f" << std::endl;
            float res = intrinsics::ScanF();
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SIN: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic sin, "
                      << "r" << (size_t)first_reg_num << std::endl;
            float first_reg = bit::getValue<float>(reg.getValue());
            float res = intrinsics::SinF(first_reg);
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::COS: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic cos, "
                      << "r" << (size_t)first_reg_num << std::endl;
            auto first_reg = bit::getValue<float>(reg.getValue());
            float res = intrinsics::CosF(first_reg);
            uint64_t res_u64 = bit::castToWritable<float>(res);
            frame->setAcc(res_u64);
            break;
        }
        case IntrinsicCode::SQRT: {
            auto reg = frame->getReg(first_reg_num);
            std::cout << "LOG_INFO: "
                      << "intrinsic sqrt, "
                      << "r" << (size_t)first_reg_num << std::endl;
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
    std::cout << "LOG_INFO: "
              << "ret" << std::endl;

    return 0;
}

}  // namespace

int runImpl(const Byte *pc, Frame *frame)
{
    dispatch_table[getOpcode(pc)](pc, frame);
    return 0;
}

}  // namespace shrimp::runtime::interpreter
