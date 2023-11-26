%option c++
%option yyclass="shrimp::assembler::Lexer"
%option yylineno

%{
#include <shrimp/assembler/lexer.hpp>
%}

WHITE_SPACE     [ \n\t\v]+
COMMENT         #.*

NUMBER          [+-]?([1-9][0-9]*\.?[0-9]*|0\.[0-9]*)

IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_.]*

%%

{WHITE_SPACE}       /* Skip */
{COMMENT}           /* Skip */

"func"              return processFunc();
","                 return processComma();
"("                 return processLeftRoundBrace();
")"                 return processRightRoundBrace();

{NUMBER}            return processNumber();
{IDENTIFIER}":"     return processLabel();
{IDENTIFIER}        return processIdentifier();

.                   return processUndef();

%%
