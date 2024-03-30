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

#include <shrimp/shrimpfile.hpp>

#include <CLI/CLI.hpp>
#include <CLI/App.hpp>

namespace shrimp {

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

    shrimpfile::File ifile {input_file};

    LOG_DEBUG(ifile.dump(), log_level);

    auto native_code = ifile.getCode();
    auto strings_info = ifile.getStringsInfo();
    auto funcs_info = ifile.getFuncsInfo();
    auto classes_info = ifile.getClassesInfo();

    runtime::ShrimpVM svm {native_code, strings_info, funcs_info, classes_info, log_level};

    return svm.runImpl();
}

}  // namespace shrimp

int main(int argc, char *argv[])
{
    return shrimp::Main(argc, argv);
}
