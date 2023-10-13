
#include <fstream>
#include <iostream>
#include <cassert>

#include <shrimp/assembler.hpp>

int yyFlexLexer::yywrap()
{
    return 1;
}

using namespace shrimp;

int main()
{
    assembler::Assembler assembler {};

    std::ofstream out {"a.out", std::ios_base::binary};

    assembler.assemble(out);

    return 0;
}
