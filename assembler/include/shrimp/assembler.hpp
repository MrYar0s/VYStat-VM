#ifndef INCLUDE_SHRIMP_ASSEMBLER_HPP
#define INCLUDE_SHRIMP_ASSEMBLER_HPP

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

class LexError : public std::runtime_error {
public:
    LexError(size_t lineno) : std::runtime_error("Lexing error on line: " + std::to_string(lineno)) {}
};

class ParseError : public std::runtime_error {
public:
    ParseError(size_t lineno) : std::runtime_error("Parse error on line: " + std::to_string(lineno)) {}
};

class UnresolvedLableError : public std::runtime_error {
public:
    UnresolvedLableError(std::string lable_name) : std::runtime_error("Unresolved lable: " + lable_name) {}
};

class DuplicatedLableError : public std::runtime_error {
public:
    DuplicatedLableError(std::string lable_name) : std::runtime_error("Duplicated lable: " + lable_name) {}
};

class Assembler final {
    class JumpInfo final {
    public:
        JumpInfo(InterfaceJump *instr_ptr, ByteOffset instr_offset, std::string dst_name)
            : instr_ptr_(instr_ptr), instr_offset_(instr_offset), dst_name_(std::move(dst_name))
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
        std::string_view getDstName() const
        {
            return dst_name_;
        }

    private:
        InterfaceJump *instr_ptr_ = nullptr;
        ByteOffset instr_offset_ = 0;
        std::string dst_name_ = "";
    };

    // Throw lexing error if condition is not true
    void assertLexError(bool cond)
    {
        if (!cond) {
            throw LexError(lexer_.lineno());
        }
    }

    // Throw parsing error if condition is not true
    void assertParseError(bool cond)
    {
        if (!cond) {
            throw ParseError(lexer_.lineno());
        }
    }

    // Parse expected register
    R8Id parseReg()
    {
        assertLexError(lexer_.yylex() == Lexer::LEXING_OK);

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::IDENTIFIER);
        assertParseError(lexer_.YYText()[0] == 'r');

        const char *reg_id_ptr = lexer_.YYText() + 1;
        const char *reg_id_end = lexer_.YYText() + lexer_.YYLeng();
        R8Id reg_id = 0;

        auto [end, ec] = std::from_chars(reg_id_ptr, reg_id_end, reg_id);

        assertParseError(ec == std::errc());
        assertParseError(reg_id_end == end);

        return reg_id;
    }

    // Parse expected int64_t immediate
    int64_t parseImmI()
    {
        assertLexError(lexer_.yylex() == Lexer::LEXING_OK);

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::NUMBER);

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
        assertLexError(lexer_.yylex() == Lexer::LEXING_OK);

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::NUMBER);

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

    // Parse expected comma
    void parseComma()
    {
        assertLexError(lexer_.yylex() == Lexer::LEXING_OK);
        assertParseError(lexer_.currLexemType() == Lexer::LexemType::COMMA);
    }

    void parseLabel()
    {
        std::string label_name = lexer_.YYText();
        // skip colon
        label_name.pop_back();

        auto [_, is_inserted] = labels_.insert({label_name, curr_offset_});

        if (!is_inserted) {
            throw DuplicatedLableError(label_name);
        }
    }

    std::string parseJumpDst()
    {
        assertLexError(lexer_.yylex() == Lexer::LEXING_OK);

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::IDENTIFIER);

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

        assertLexError(lexer_.yylex() == Lexer::LEXING_OK);

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::IDENTIFIER);

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
                parseComma();
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::SCAN_I32:
            case IntrinsicCode::SCAN_F:
                return {0, 0, 0, 0};

            case IntrinsicCode::SIN:
                parseComma();
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::COS:
                parseComma();
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::SQRT:
                parseComma();
                return {parseReg(), 0, 0, 0};

            default:
                assert(0);
        }
    }

    template <InstrOpcode>
    void parseInstr();

    void first_pass();
    void reslove_jumps()
    {
        for (auto &&jump : jumps_) {
            auto lable_name = jump.getDstName();
            auto label_it = labels_.find(lable_name.data());
            if (label_it == labels_.end()) {
                throw UnresolvedLableError(lable_name.data());
            }

            auto label_offset = label_it->second;

            jump.getInstrPtr()->setOffset(label_offset - jump.getInstrOffset());
        }
    }

    void write(std::ofstream &out)
    {
        for (auto &&instr_ptr : instrs_) {
            const char *bin_code = reinterpret_cast<const char *>(instr_ptr->getBinCode());
            out.write(bin_code, instr_ptr->getByteSize());
        }
    }

public:
    void assemble(std::ofstream &out)
    {
        first_pass();
        reslove_jumps();
        write(out);
    }

private:
    Lexer lexer_;

    ByteOffset curr_offset_ = 0;

    // Parsed instructions
    std::vector<std::unique_ptr<InterfaceInstr>> instrs_ {};
    // Parsed jumps
    std::vector<JumpInfo> jumps_ {};
    // Parsed labels: [Label name -> offset]
    std::unordered_map<std::string, ByteOffset> labels_ {};
};

}  // namespace shrimp::assembler

#include <shrimp/assembler/parser.gen.hpp>

#endif  // INCLUDE_SHRIMP_ASSEMBLER_HPP
