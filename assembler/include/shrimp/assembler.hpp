#ifndef INCLUDE_SHRIMP_ASSEMBLER_HPP
#define INCLUDE_SHRIMP_ASSEMBLER_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>
#include <charconv>

#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/assembler/instr.gen.hpp>
#include <shrimp/assembler/lexer.hpp>

namespace shrimp::assembler {

class Error : public std::runtime_error {
public:
    Error(std::string descr, size_t lineno)
        : std::runtime_error("Assembler error at line " + std::to_string(lineno) + " :" + descr)
    {
    }
};

class Assembler final {
    class JumpInfo final {
    public:
        JumpInfo(InterfaceJump *instr_ptr, ByteOffset instr_offset, std::string dst_name, int lineno)
            : instr_ptr_(instr_ptr), instr_offset_(instr_offset), dst_name_(std::move(dst_name)), lineno_(lineno)
        {
        }

        auto *getInstrPtr() const
        {
            return instr_ptr_;
        }
        auto getInstrOffset() const
        {
            return instr_offset_;
        }
        std::string getDstName() const
        {
            return dst_name_;
        }
        auto getLineno() const
        {
            return lineno_;
        }

    private:
        InterfaceJump *instr_ptr_ = nullptr;
        ByteOffset instr_offset_ = 0;
        std::string dst_name_ = "";
        int lineno_ = 0;
    };

    class FuncInfo final {
    public:
        FuncInfo(std::vector<std::string> args) : args_(args) {}

        auto getArgs() const noexcept
        {
            return args_;
        }

        auto &instrs() noexcept
        {
            return instrs_;
        }
        auto &jumps() noexcept
        {
            return jumps_;
        }
        auto &labels() noexcept
        {
            return labels_;
        }

    private:
        // Function arguments aliases
        std::vector<std::string> args_ {};

        // Parsed instructions
        std::vector<std::unique_ptr<InterfaceInstr>> instrs_ {};
        // Parsed jumps
        std::vector<JumpInfo> jumps_ {};
        // Parsed labels: [Label name -> offset]
        std::unordered_map<std::string, ByteOffset> labels_ {};
    };

    // Try to get next lexem. Throw Error if failed
    void expectLexem()
    {
        if (lexer_.yylex() != Lexer::LEXING_OK) {
            throw Error("Failed to get next lexem", lexer_.lineno());
        }
    }

    // Try to get next lexem. Throw Error if failed
    void expectLexem(Lexer::LexemType lexem_type)
    {
        if (lexer_.yylex() != Lexer::LEXING_OK || lexer_.currLexemType() != lexem_type) {
            throw Error("Failed to get next lexem", lexer_.lineno());
        }
    }

    // Throw Error if condition is not true
    void assertParseError(bool cond)
    {
        if (!cond) {
            throw Error("Parse error", lexer_.lineno());
        }
    }

    // Parse expected register
    R8Id parseReg()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        R8Id reg_id = 0;
        const char *reg_name = lexer_.YYText();

        const auto &args = curr_func_->getArgs();
        bool found_arg = std::any_of(args.cbegin(), args.cend(), [&](const std::string &arg) {
            --reg_id;
            return std::strcmp(arg.c_str(), reg_name) == 0;
        });

        if (found_arg) {
            return reg_id;
        }

        assertParseError(std::toupper(lexer_.YYText()[0]) == 'R');

        const char *reg_id_ptr = lexer_.YYText() + 1;
        const char *reg_id_end = lexer_.YYText() + lexer_.YYLeng();

        auto [end, ec] = std::from_chars(reg_id_ptr, reg_id_end, reg_id);

        assertParseError(ec == std::errc());
        assertParseError(reg_id_end == end);

        return reg_id;
    }

    // Parse expected int64_t immediate
    int64_t parseImmI()
    {
        expectLexem(Lexer::LexemType::NUMBER);

        int64_t immi = 0;
        const char *immi_end = lexer_.YYText() + lexer_.YYLeng();

        auto [end, ec] = std::from_chars(lexer_.YYText(), immi_end, immi);

        assertParseError(ec == std::errc());
        assertParseError(immi_end == end);

        return immi;
    }

    // Parse expected floating point value
    double parseFP()
    {
        expectLexem(Lexer::LexemType::NUMBER);

        double value = 0;
        const char *value_end = lexer_.YYText() + lexer_.YYLeng();

        auto [end, ec] = std::from_chars(lexer_.YYText(), value_end, value);

        assertParseError(ec == std::errc());
        assertParseError(value_end == end);

        return value;
    }

    // Parse expected float immediate
    uint32_t parseImmF()
    {
        float value = parseFP();
        return std::bit_cast<uint32_t>(value);
    }

    // Parse expected double immediate
    uint64_t parseImmD()
    {
        double value = parseFP();
        return std::bit_cast<uint64_t>(value);
    }

    // Add lexed label to current function labels
    void addLexedLabel()
    {
        assert(lexer_.currLexemType() == Lexer::LexemType::LABEL);

        std::string label_name = lexer_.YYText();
        // skip colon
        label_name.pop_back();

        auto [_, is_inserted] = curr_func_->labels().insert({label_name, curr_offset_});

        if (!is_inserted) {
            throw Error("Lable redeclaration: " + label_name, lexer_.lineno());
        }
    }

    // Parse jump destination label name
    std::string parseJumpDst()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);
        return lexer_.YYText();
    }

    IntrinsicCode parseIntrinsicName()
    {
        static std::unordered_map<std::string, IntrinsicCode> intrinsics = {{"PRINT.I32", IntrinsicCode::PRINT_I32},
                                                                            {"PRINT.F", IntrinsicCode::PRINT_F},
                                                                            {"SCAN.I32", IntrinsicCode::SCAN_I32},
                                                                            {"SCAN.F", IntrinsicCode::SCAN_F},
                                                                            {"SIN", IntrinsicCode::SIN},
                                                                            {"COS", IntrinsicCode::COS},
                                                                            {"SQRT", IntrinsicCode::SQRT}};

        expectLexem(Lexer::LexemType::IDENTIFIER);

        std::string intrinsic_name = lexer_.YYText();
        std::transform(intrinsic_name.begin(), intrinsic_name.end(), intrinsic_name.begin(), ::toupper);

        auto it = intrinsics.find(intrinsic_name);
        assertParseError(it != intrinsics.end());

        return it->second;
    }

    std::array<R8Id, 4> parseIntrinsicArgs(IntrinsicCode code)
    {
        switch (code) {
            case IntrinsicCode::PRINT_I32:
            case IntrinsicCode::PRINT_F:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::SCAN_I32:
            case IntrinsicCode::SCAN_F:
                return {0, 0, 0, 0};

            case IntrinsicCode::SIN:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::COS:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::SQRT:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            default:
                assert(0);
        }
    }

    FuncId parseCallFuncId()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        auto it = func_name_to_id_.find(lexer_.YYText());
        assertParseError(it != func_name_to_id_.end());

        return it->second;
    }

    void parseFuncDecl()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        func_name_to_id_.insert({lexer_.YYText(), funcs_.size()});

        expectLexem(Lexer::LexemType::LEFT_ROUND_BRACE);

        std::vector<std::string> args {};

        expectLexem();

        if (lexer_.currLexemType() == Lexer::LexemType::IDENTIFIER) {
            args.push_back(lexer_.YYText());

            while (expectLexem(), lexer_.currLexemType() == Lexer::LexemType::COMMA) {
                expectLexem(Lexer::LexemType::IDENTIFIER);
                args.push_back(lexer_.YYText());
            }
        }

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::RIGHT_ROUND_BRACE);

        static constexpr size_t MAX_FUNC_ARGS_NUMBER = 4;
        assertParseError(args.size() <= MAX_FUNC_ARGS_NUMBER);

        funcs_.emplace_back(args);
        curr_func_ = &funcs_.back();
    }

    // Generated
    int parseFuncBody();

    // Generated
    template <InstrOpcode>
    void parseInstr();

    void firstPass()
    {
        int lexing_status = lexer_.yylex();

        while (lexing_status == Lexer::LEXING_OK) {
            // Parse next function
            assertParseError(lexer_.currLexemType() == Lexer::LexemType::FUNC);

            parseFuncDecl();
            lexing_status = parseFuncBody();
        }
    }

    void resloveJumps()
    {
        for (auto &&func : funcs_) {
            for (auto &&jump : func.jumps()) {
                auto lable_name = jump.getDstName();
                auto label_it = func.labels().find(lable_name.c_str());
                if (label_it == func.labels().end()) {
                    throw Error("Unresolved lable" + lable_name, jump.getLineno());
                }

                auto label_offset = label_it->second;

                jump.getInstrPtr()->setOffset(label_offset - jump.getInstrOffset());
            }
        }
    }

    void write(std::ofstream &out)
    {
        for (auto &&func : funcs_) {
            for (auto &&instr_ptr : func.instrs()) {
                const char *bin_code = reinterpret_cast<const char *>(instr_ptr->getBinCode());
                out.write(bin_code, instr_ptr->getByteSize());
            }
        }
    }

public:
    void assemble(std::ifstream &in, std::ofstream &out)
    {
        lexer_.switch_streams(&in);

        firstPass();
        resloveJumps();
        write(out);
    }

private:
    Lexer lexer_;

    ByteOffset curr_offset_ = 0;

    std::unordered_map<std::string, FuncId> func_name_to_id_ {};

    std::vector<FuncInfo> funcs_ {};
    FuncInfo *curr_func_ = nullptr;
};

template <>
inline void Assembler::parseInstr<InstrOpcode::INTRINSIC>()
{
    auto &&instrs = curr_func_->instrs();

    auto intrinsic_code_enum = parseIntrinsicName();
    auto [arg0, arg1, arg2, arg3] = parseIntrinsicArgs(intrinsic_code_enum);
    auto intrinsic_code = static_cast<uint8_t>(intrinsic_code_enum);

    instrs.push_back(std::make_unique<Instr<InstrOpcode::INTRINSIC>>(intrinsic_code, arg0, arg1, arg2, arg3));

    curr_offset_ += instrs.back()->getByteSize();
}

}  // namespace shrimp::assembler

#include <shrimp/assembler/parser.gen.hpp>

#endif  // INCLUDE_SHRIMP_ASSEMBLER_HPP
