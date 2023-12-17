#include <shrimp/lang2shrimp.hpp>

#include <CLI/CLI.hpp>
#include "CLI/App.hpp"

int main(int argc, char *argv[])
{
    CLI::App app("Shrimp frontend, based on recursive descent parser");

    std::string output_file {};
    auto *output_arg = app.add_option("-o,--out", output_file, "Output file");
    output_arg->default_str("a.out");

    std::string input_file {};
    auto *input_arg = app.add_option("--in", input_file, "Input file");
    input_arg->required();

    CLI11_PARSE(app, argc, argv);

    shrimp::Compiler compiler {input_file, output_file};
    compiler.run();
}