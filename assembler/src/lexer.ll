%option c++
%option yyclass="shrimp::assembler::Lexer"
%option yylineno

%{
#include <shrimp/assembler/lexer.hpp>
%}

WHITE_SPACE     [ \n\t\v]+
COMMENT         #.*

NUMBER          [+-]?([1-9][0-9]*\.?[0-9]*|0\.[0-9]*|0)

IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_.]*

STRING          \"[^\"]*\"

%%

{WHITE_SPACE}       /* Skip */
{COMMENT}           /* Skip */

"class"             return processClass();
"func"              return processFunc();
","                 return processComma();
"("                 return processLeftRoundBrace();
")"                 return processRightRoundBrace();

{STRING}            return processString();
{NUMBER}            return processNumber();
{IDENTIFIER}":"     return processLabel();
{IDENTIFIER}        return processIdentifier();

.                   return processUndef();

%%
