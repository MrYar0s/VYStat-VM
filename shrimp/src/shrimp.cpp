#include <sys/types.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>

#include <shrimp/runtime/shrimp_vm.hpp>

namespace shrimp {

std::string getFileName(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Wrong number of arguments\n" << std::endl;
        std::abort();
    }
    // File name must be placed on the last position (need to specify it)
    char *file_name_arg = argv[argc - 1];
    return file_name_arg;
}

std::vector<uint64_t> readFromFile(std::string file_name)
{
    FILE *file = fopen(file_name.data(), "rb");
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0L, SEEK_SET);
    uint64_t *buf = (uint64_t *)calloc(sizeof(uint64_t), len);
    fread(buf, sizeof(uint64_t), len, file);
    std::vector<uint64_t> code;
    code.assign(buf, buf + len);
    free(buf);
    fclose(file);
    return code;
}

int Main(int argc, char *argv[])
{
    std::string file_name = getFileName(argc, argv);
    auto native_code = readFromFile(file_name);
    shrimp::ShrimpVM svm {native_code};
    shrimp::Runtime runtime = shrimp::Runtime(&svm);
    svm.setRuntime(&runtime);
    runtime.runImpl();
    return 0;
}

}  // namespace shrimp

int main(int argc, char *argv[])
{
    return shrimp::Main(argc, argv);
}