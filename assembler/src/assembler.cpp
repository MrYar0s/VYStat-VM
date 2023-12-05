
#include <fstream>
#include <iostream>
#include <cassert>

#include <CLI/CLI.hpp>

#include <shrimp/assembler.hpp>
#include "CLI/App.hpp"

using namespace shrimp;

int yyFlexLexer::yywrap()
{
    return 1;
}

int main(int argc, char **argv)
{
    CLI::App app("Shrimp assembler");

    std::string output_file {};
    auto *output_arg = app.add_option("-o,--out", output_file, "Output file");
    output_arg->default_str("a.out");

    std::string input_file {};
    auto *input_arg = app.add_option("--in", input_file, "Input file");
    input_arg->required();

    CLI11_PARSE(app, argc, argv);

    assembler::Assembler assembler {};

    std::ofstream out {output_file, std::ios_base::binary};

    assembler.assemble(input_file, output_file);

    return 0;
}
