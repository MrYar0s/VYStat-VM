#include <shrimp/lexer.hpp>

#include <locale>
#include <iostream>

void Lexer::run()
{
    while (program_it_ != program_end_) {
        while (isspace(*program_it_)) {
            ++program_it_;
        }

        if (program_it_ == program_end_) {
            tokens_.emplace_back("", TokenType::END);
            break;
        }

        const char currentChar = *program_it_;

        switch (currentChar) {
            case '{':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::OPEN_FIG_BRACKET);
                break;

            case '}':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::CLOSE_FIG_BRACKET);
                break;

            case '(':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::OPEN_BRACKET);
                break;

            case ')':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::CLOSE_BRACKET);
                break;

            case ',':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::COMMA);
                break;

            case ';':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::SEMICOLON);
                break;

            case '+':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::PLUS);
                break;

            case '*':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::MUL);
                break;

            case '-':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::MINUS);
                break;

            case '/':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::DIVIDE);
                break;

            case '=':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::EQUAL);
                break;

            default:
                if (isalpha(currentChar)) {
                    curr_ident_ = currentChar;
                    while ((++program_it_ != program_end_) && isalnum(*program_it_)) {
                        curr_ident_ += *program_it_;
                    }
                    if (curr_ident_ == "function") {
                        tokens_.emplace_back(curr_ident_, TokenType::FUNCTION);
                    } else if (curr_ident_ == "if") {
                        tokens_.emplace_back(curr_ident_, TokenType::IF);
                    } else if (curr_ident_ == "return") {
                        tokens_.emplace_back(curr_ident_, TokenType::RETURN);
                    } else if (curr_ident_ == "int") {
                        tokens_.emplace_back(curr_ident_, TokenType::TYPE);
                    } else {
                        tokens_.emplace_back(curr_ident_, TokenType::IDENTIFIER);
                    }
                } else if (isdigit(currentChar)) {
                    std::string numberStr;
                    while (isdigit(*program_it_)) {
                        numberStr.append(std::string {*program_it_});
                        ++program_it_;
                    }
                    tokens_.emplace_back(numberStr, TokenType::NUMBER);
                } else {
                    tokens_.emplace_back(currentChar, TokenType::UNKNOWN);
                }
                break;
        }
    }
    tokens_.emplace_back("", TokenType::END);
}
