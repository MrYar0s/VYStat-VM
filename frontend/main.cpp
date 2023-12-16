#include <shrimp/lang2shrimp.hpp>

int main()
{
    std::string programFile = "test.lang";
    shrimp::Compiler compiler {programFile};
    compiler.run();
}