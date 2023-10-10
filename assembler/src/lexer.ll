%option c++
%option yyclass="shrimp::Lexer"

%{
#include <shrimp/lexer.hpp>
%}

WHITE_SPACE     [ \t\n\v]+
COMMENT         #.*

NUMBER          [+-]?([1-9][0-9]*\.?[0-9]*|0\.[0-9]*)

COMMA           ,
COLON           :

IDENTIFIER      [a-zA-Z_][a-zA-Z0-9_.]*

%%

{WHITE_SPACE}       /* Skip */
{COMMENT}           /* Skip */

{NUMBER}            return processNumber();
{COMMA}             return processComma();
{COLON}             return processColon();
{IDENTIFIER}        return processIdentifier();

.                   return processUndef();

%%
