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
#include <utility>
#include <vector>
#include <unordered_map>

#include <shrimp/common/inst_opcode.gen.hpp>
#include <shrimp/assembler/inst.gen.hpp>

#include <shrimp/lexer.hpp>

namespace shrimp {
namespace assembler {

struct LexError : std::runtime_error {
    LexError(size_t lineno) : std::runtime_error("Lexing error on line: " + std::to_string(lineno)) {}
};

struct ParseError : std::runtime_error {
    ParseError(size_t lineno) : std::runtime_error("Parse error on line: " + std::to_string(lineno)) {}
};

struct LableError : std::runtime_error {
    LableError(std::string lable_name) : std::runtime_error("Unresolved lable: " + lable_name) {}
};

class Assembler final {
    Lexer m_lexer;

    InstWordNumber m_curr_inst_word_number = 0;

    std::vector<std::unique_ptr<InterfaceInst>> m_insts {};
    std::vector<std::pair<InterfaceJump *, std::string>> m_jumps {};
    std::unordered_map<std::string, InstWordNumber> m_labels {};

    void assertLexError(bool cond)
    {
        if (!cond) {
            throw LexError(m_lexer.lineno());
        }
    }

    void assertParseError(bool cond)
    {
        if (!cond) {
            throw ParseError(m_lexer.lineno());
        }
    }

    uint64_t parseReg() {
        assertLexError(m_lexer.yylex() == Lexer::LEXING_OK);

        assertParseError(m_lexer.currLexemType() == Lexer::LexemType::IDENTIFIER);
        assertParseError(m_lexer.YYText()[0] == 'r');

        char *end = nullptr;
        int reg_id = std::strtol(m_lexer.YYText() + 1, &end, 10);

        assertParseError(end == m_lexer.YYText() + m_lexer.YYLeng());

        return reg_id;
    }

    int64_t parseImmI() {
        assertLexError(m_lexer.yylex() == Lexer::LEXING_OK);

        assertParseError(m_lexer.currLexemType() == Lexer::LexemType::NUMBER);

        char *end = nullptr;
        int64_t imm = std::strtoll(m_lexer.YYText(), &end, 10);

        assertParseError(end == m_lexer.YYText() + m_lexer.YYLeng());

        return imm;
    }

    double parseImmD() {
        assertLexError(m_lexer.yylex() == Lexer::LEXING_OK);

        assertParseError(m_lexer.currLexemType() == Lexer::LexemType::NUMBER);

        char *end = nullptr;
        double imm = std::strtod(m_lexer.YYText(), &end);

        assertParseError(end == m_lexer.YYText() + m_lexer.YYLeng());

        return imm;
    }

    void parseComma() {
        assertLexError(m_lexer.yylex() == Lexer::LEXING_OK);
        assertParseError(m_lexer.currLexemType() == Lexer::LexemType::COMMA);
    }

    void parseColon() {
        assertLexError(m_lexer.yylex() == Lexer::LEXING_OK);
        assertParseError(m_lexer.currLexemType() == Lexer::LexemType::COLON);
    }

    void parseLabel() {
        std::string label_name = m_lexer.YYText();
        parseColon();

        m_labels.insert(std::make_pair(label_name, m_curr_inst_word_number));
    }

    void parseJumpDst() {
        assertLexError(m_lexer.yylex() == Lexer::LEXING_OK);

        assertParseError(m_lexer.currLexemType() == Lexer::LexemType::IDENTIFIER);

        std::string dst_name = m_lexer.YYText();

        m_labels.insert(std::make_pair(dst_name, m_curr_inst_word_number));
    }

    template<InstOpcode>
    void parseInst();

    void first_pass();
    void reslove_jumps() {
        for (auto &&jump : m_jumps) {
            auto &lable_name = jump.second;
            auto it = m_labels.find(lable_name);
            if (it == m_labels.end()) {
                throw LableError(lable_name);
            }

            jump.first->setOffset(it->second);
        }
    }

    void write(std::ofstream &out) {
        for (auto &&inst_ptr : m_insts) {
            const char *bin_code = reinterpret_cast<const char*>(inst_ptr->getBinCode());
            int size = inst_ptr->getWordSize() * sizeof(InstWord);
            out.write(bin_code, size);
        }
    }

public:
    void assemble(std::ofstream &out) {
        first_pass();
        reslove_jumps();
        write(out);
    }
};

}  // namespace assembler
}  // namespace shrimp

#include <shrimp/assembler/parser.gen.hpp>

#endif  // INCLUDE_SHRIMP_ASSEMBLER_HPP
