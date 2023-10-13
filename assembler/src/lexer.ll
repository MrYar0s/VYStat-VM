%option c++
%option yyclass="shrimp::Lexer"
%option yylineno

%{
#include <shrimp/lexer.hpp>
%}

WHITE_SPACE     [ \n\t\v]+
COMMENT         #.*

NUMBER          [+-]?([1-9][0-9]*\.?[0-9]*|0\.[0-9]*)

COMMA           ,
COLON           :

IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_.]*

%%

{WHITE_SPACE}       /* Skip */
{COMMENT}           /* Skip */

{COMMA}             return processComma();

{NUMBER}            return processNumber();
{IDENTIFIER}{COLON} return processLabel();
{IDENTIFIER}        return processIdentifier();

.                   return processUndef();

%%
