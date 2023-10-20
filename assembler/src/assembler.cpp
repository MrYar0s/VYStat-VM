
#include <fstream>
#include <iostream>
#include <cassert>

#include <shrimp/assembler.hpp>

using namespace shrimp;

int yyFlexLexer::yywrap()
{
    return 1;
}

int main()
{
    assembler::Assembler assembler {};

    std::ofstream out {"a.out", std::ios_base::binary};

    assembler.assemble(out);

    return 0;
}
