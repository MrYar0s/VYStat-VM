#ifndef FRONTEND_INCLUDE_SHRIMP_LEXER_HPP
#define FRONTEND_INCLUDE_SHRIMP_LEXER_HPP

#include <vector>
#include <string>

enum class TokenType {
    // Keywords
    FUNCTION,
    IF,
    RETURN,
    NUMBER,
    TYPE,

    // Operators
    EQUAL,
    DIVIDE,
    MINUS,
    MUL,
    PLUS,

    // Misc
    COMMA,
    COMMENT,
    IDENTIFIER,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    OPEN_FIG_BRACKET,
    CLOSE_FIG_BRACKET,
    SEMICOLON,

    // End of file
    END,

    // Unknown
    UNKNOWN
};

struct Token {
    Token(std::string val, TokenType tp) : value {val}, type {tp} {}
    Token(const char val, TokenType tp) : value(std::string {val}), type(tp) {}
    std::string value;
    TokenType type;
};

class Lexer {
public:
    Lexer() = default;

    explicit Lexer(const std::string &programStr)
        : program_ {programStr}, program_it_ {program_.cbegin()}, program_end_ {program_.cend()} {};

    Token getNextToken();

    void run();

    const auto &getTokens() const
    {
        return tokens_;
    }

private:
    std::vector<Token> tokens_ {};
    std::string program_;
    std::string::const_iterator program_it_;
    std::string::const_iterator program_end_;

    int curr_int_number_;
    float curr_float_number_;
    std::string curr_ident_;
};

#endif  // FRONTEND_INCLUDE_SHRIMP_LEXER_HPP
