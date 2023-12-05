#ifndef SHRIMP_ASSEMBLER_LEXER_HPP
#define SHRIMP_ASSEMBLER_LEXER_HPP

#ifndef yyFlexLexer
#include <FlexLexer.h>
#endif

namespace shrimp::assembler {

class Lexer final : public yyFlexLexer {
public:
    enum class LexemType { NUMBER, COMMA, LABEL, IDENTIFIER, FUNC, LEFT_ROUND_BRACE, RIGHT_ROUND_BRACE, STRING };

    static constexpr int LEXING_OK = 1;
    static constexpr int LEXING_ERROR_CODE = -1;

private:
    int processNumber() noexcept
    {
        curr_lexem_type_ = LexemType::NUMBER;
        return LEXING_OK;
    }

    int processFunc() noexcept
    {
        curr_lexem_type_ = LexemType::FUNC;
        return LEXING_OK;
    }

    int processLeftRoundBrace() noexcept
    {
        curr_lexem_type_ = LexemType::LEFT_ROUND_BRACE;
        return LEXING_OK;
    }

    int processRightRoundBrace() noexcept
    {
        curr_lexem_type_ = LexemType::RIGHT_ROUND_BRACE;
        return LEXING_OK;
    }

    int processComma() noexcept
    {
        curr_lexem_type_ = LexemType::COMMA;
        return LEXING_OK;
    }

    int processLabel() noexcept
    {
        curr_lexem_type_ = LexemType::LABEL;
        return LEXING_OK;
    }

    int processIdentifier() noexcept
    {
        curr_lexem_type_ = LexemType::IDENTIFIER;
        return LEXING_OK;
    }

    int processString() noexcept
    {
        curr_lexem_type_ = LexemType::STRING;
        return LEXING_OK;
    }

    int processUndef() noexcept
    {
        return LEXING_ERROR_CODE;
    }

public:
    int yylex() override;

    auto currLexemType() const noexcept
    {
        return curr_lexem_type_;
    }

private:
    LexemType curr_lexem_type_;
};

}  // namespace shrimp::assembler

#endif  // SHRIMP_ASSEMBLER_LEXER_HPP
