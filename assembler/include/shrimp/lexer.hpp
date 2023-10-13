#ifndef INCLUDE_SHRIMP_LEXER_HPP
#define INCLUDE_SHRIMP_LEXER_HPP

#ifndef yyFlexLexer
#include <FlexLexer.h>
#endif

namespace shrimp {

struct Lexer final : public yyFlexLexer {
    enum class LexemType {
        NUMBER,
        COMMA,
        COLON,
        IDENTIFIER,
        NEW_LINE
    };

    static constexpr int LEXING_OK = 1;
    static constexpr int LEXING_ERROR_CODE = -1;

  private:
    LexemType m_curr_lexem_type;

    int processNumber() noexcept {
        m_curr_lexem_type = LexemType::NUMBER;
        return LEXING_OK;
    }

    int processComma() noexcept {
        m_curr_lexem_type = LexemType::COMMA;
        return LEXING_OK;
    }

    int processColon() noexcept {
        m_curr_lexem_type = LexemType::COLON;
        return LEXING_OK;
    }

    int processIdentifier() noexcept {
        m_curr_lexem_type = LexemType::IDENTIFIER;
        return LEXING_OK;
    }

    int processNewLine() noexcept {
        m_curr_lexem_type = LexemType::NEW_LINE;
        return LEXING_OK;
    }

    int processUndef() noexcept {
        return LEXING_ERROR_CODE;
    }

  public:
    int yylex() override;

    auto currLexemType() const noexcept {
        return m_curr_lexem_type;
    }
};

} // namespace shrimp

#endif  // INCLUDE_SHRIMP_LEXER_HPP
