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

            case '[':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::OPEN_SQUARE_BRACKET);
                break;

            case ']':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::CLOSE_SQUARE_BRACKET);
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
                if (*program_it_ == '=') {
                    program_it_++;
                    tokens_.emplace_back("==", TokenType::IS_EQUAL);
                    break;
                }
                tokens_.emplace_back(currentChar, TokenType::EQUAL);
                break;

            case '>':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::IS_GREATER);
                break;

            case '<':
                ++program_it_;
                tokens_.emplace_back(currentChar, TokenType::IS_LESS);
                break;

            case '\"':
                ++program_it_;
                curr_ident_ = *program_it_;
                while (++program_it_ != program_end_ && *program_it_ != '\"') {
                    curr_ident_ += *program_it_;
                }
                program_it_++;
                tokens_.emplace_back(curr_ident_, TokenType::STRING);
                break;

            default:
                if (isalpha(currentChar)) {
                    curr_ident_ = currentChar;
                    while ((++program_it_ != program_end_) && (isalnum(*program_it_) || *program_it_ == '.')) {
                        curr_ident_ += *program_it_;
                    }
                    if (curr_ident_ == "intrinsic.scan") {
                        tokens_.emplace_back(curr_ident_, TokenType::SCAN);
                    } else if (curr_ident_ == "intrinsic.sqrt") {
                        tokens_.emplace_back(curr_ident_, TokenType::SQRT);
                    } else if (curr_ident_ == "intrinsic.print") {
                        tokens_.emplace_back(curr_ident_, TokenType::PRINT);
                    } else if (curr_ident_ == "intrinsic.concat") {
                        tokens_.emplace_back(curr_ident_, TokenType::CONCAT);
                    } else if (curr_ident_ == "intrinsic.substr") {
                        tokens_.emplace_back(curr_ident_, TokenType::SUBSTR);
                    } else if (curr_ident_ == "function") {
                        tokens_.emplace_back(curr_ident_, TokenType::FUNCTION);
                    } else if (curr_ident_ == "if") {
                        tokens_.emplace_back(curr_ident_, TokenType::IF);
                    } else if (curr_ident_ == "for") {
                        tokens_.emplace_back(curr_ident_, TokenType::FOR);
                    } else if (curr_ident_ == "return") {
                        tokens_.emplace_back(curr_ident_, TokenType::RETURN);
                    } else if (curr_ident_ == "int" || curr_ident_ == "float" || curr_ident_ == "string") {
                        tokens_.emplace_back(curr_ident_, TokenType::TYPE);
                    } else {
                        tokens_.emplace_back(curr_ident_, TokenType::IDENTIFIER);
                    }
                } else if (isdigit(currentChar)) {
                    std::string numberStr;
                    while (isdigit(*program_it_) || *program_it_ == '.') {
                        numberStr.append(std::string {*program_it_});
                        ++program_it_;
                    }
                    tokens_.emplace_back(numberStr, TokenType::NUMBER);
                } else {
                    std::cout << "Unknown token: " << currentChar << std::endl;
                    tokens_.emplace_back(currentChar, TokenType::UNKNOWN);
                    std::abort();
                }
                break;
        }
    }
    tokens_.emplace_back("", TokenType::END);
}
