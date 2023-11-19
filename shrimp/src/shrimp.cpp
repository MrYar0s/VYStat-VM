#include <sys/types.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>

#include <shrimp/common/logger.hpp>

#include <shrimp/runtime/shrimp_vm.hpp>

#include <CLI/CLI.hpp>
#include <CLI/App.hpp>

namespace shrimp {

std::vector<Byte> readFromFile(std::string file_name)
{
    FILE *file = fopen(file_name.data(), "rb");
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0L, SEEK_SET);
    Byte *buf = (Byte *)calloc(sizeof(Byte), len);
    fread(buf, sizeof(Byte), len, file);
    std::vector<Byte> code;
    code.assign(buf, buf + len);
    free(buf);
    fclose(file);
    return code;
}

int Main(int argc, char *argv[])
{
    CLI::App app("Shrimp VM");

    std::string log_level_str {};
    auto *log_level_cli = app.add_option("--log-level", log_level_str, "Log level [error, info, debug, none]");
    log_level_cli->default_str("none");

    std::string input_file {};
    auto *input_arg = app.add_option("--in", input_file, "Input file");
    input_arg->required();

    CLI11_PARSE(app, argc, argv);

    LogLevel log_level = getLogLevelByString(log_level_str);
    auto native_code = readFromFile(input_file);
    runtime::ShrimpVM svm {native_code, log_level};

    return svm.runImpl();
}

}  // namespace shrimp

int main(int argc, char *argv[])
{
    return shrimp::Main(argc, argv);
}
