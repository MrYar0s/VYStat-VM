
#include <iostream>
#include <cassert>

#include <shrimp/lexer.hpp>

int yyFlexLexer::yywrap() {
    return 1;
}

using namespace shrimp;

int main() {
    Lexer lexer {};

    while(lexer.yylex() == Lexer::LEXING_OK) {

        switch(lexer.currLexemType()) {
            case Lexer::LexemType::COLON:
                std::cout << "<:> ";
                break;
            case Lexer::LexemType::COMMA:
                std::cout << "<,> ";
                break;
            case Lexer::LexemType::IDENTIFIER:
                std::cout << "<id: " << lexer.YYText() << "> ";
                break;
            case Lexer::LexemType::NUMBER:
                std::cout << "<num: " << lexer.YYText() << "> ";
                break;
            default:
                assert(0);
        }
    }

    return 0;
}
