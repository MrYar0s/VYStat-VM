#ifndef INCLUDE_SHRIMP_FILE_HPP
#define INCLUDE_SHRIMP_FILE_HPP

#include <sys/types.h>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <shrimp/common/types.hpp>

#include <iostream>

namespace shrimp::shrimpfile {

constexpr size_t MAGIC_SIZE = 8;
constexpr size_t FILENAME_SIZE = 32;

class File final {
public:
    enum Headers { CODE = 0, LITERALS, FUNCTIONS, CLASSES, HEADERS_NUM };

    File() = default;
    explicit File(const std::string &src_file_name, const std::string &bin_file_name);
    explicit File(const std::string &bin_file_name);

    void fillHeaders();

    void serialize();
    static void read();

    struct SegmentsHeader {
        uint32_t size = 0;           // Size of next segment (maybe header in hard cases)
        uint32_t offset_from_start;  // Offset to next segment from start
    } __attribute__((packed));

    struct Header_t {
        uint8_t magic[MAGIC_SIZE] = {0};
        uint8_t bin_file_name[FILENAME_SIZE] = {0};
        uint32_t headers_num = Headers::HEADERS_NUM;
        uint32_t file_header_size = sizeof(Header_t);
        SegmentsHeader headers[Headers::HEADERS_NUM];
    } __attribute__((packed));

    struct EntityHeader_t {
        uint32_t num = 0;
        uint32_t header_size = sizeof(EntityHeader_t);
    } __attribute__((packed));

    Header_t FileHeader;
    EntityHeader_t FileStringHeader;
    EntityHeader_t FileFuncHeader;
    EntityHeader_t FileClassHeader;

    struct FileField {
        FieldId id = 0;
        uint8_t is_ref = 0;
        uint64_t size = 0;
        uint64_t offset = 0;
        uint64_t name_size = 0;
        std::string name = "";
    };

    struct FileFunction {
        FuncId id = 0;
        ByteOffset func_start = 0;
        uint64_t name_size = 16;
        uint8_t num_of_args = 4;
        uint16_t num_of_vregs = 256;
        std::string name = "";
    };

    struct FileClass {
        ClassId id = 0;
        uint64_t size = 0;
        uint64_t name_size = 0;
        std::string name = "";
        uint32_t num_of_fields = 0;
        std::vector<FileField> fields;
    };

    struct FileString {
        StrId id = 0;
        uint64_t str_size = 0;
        std::string str = "";
    };

    void writeBytes(const char *bin_code, size_t size);
    void writeString(const std::string &str, StrId str_id);
    void writeFunction(const FileFunction &func);
    void writeClass(const FileClass &klass);
    std::string dump();

    auto getCode() noexcept
    {
        return Code;
    }

    auto getStringsInfo() noexcept
    {
        return Strings;
    }

    auto getFuncsInfo() noexcept
    {
        return Functions;
    }

    auto &getClassesInfo() noexcept
    {
        return Classes;
    }

private:
    void fillCodeHeader();
    void fillStringsHeader();
    void fillFunctionsHeader();
    void fillClassesHeader();

    std::string bin_file_path_;
    std::vector<FileString> Strings;
    std::vector<FileFunction> Functions;
    std::vector<FileClass> Classes;
    std::vector<Byte> Code;

    void serializeCode(std::FILE *out);
    void serializeFileHeader(std::FILE *out);
    void serializeStrings(std::FILE *out);
    void serializeFunctions(std::FILE *out);
    void serializeHeaders(std::FILE *out);
    void serializeClasses(std::FILE *out);

    void dumpFileHeader(std::stringstream &ss);
    void dumpCodeHeader(std::stringstream &ss);
    void dumpCode(std::stringstream &ss);
    void dumpStringHeader(std::stringstream &ss);
    void dumpString(std::stringstream &ss);
    void dumpFunctionHeader(std::stringstream &ss);
    void dumpFunction(std::stringstream &ss);
    void dumpClassHeader(std::stringstream &ss);
    void dumpClasses(std::stringstream &ss);
};

constexpr size_t HEADERS_NUM = File::Headers::HEADERS_NUM;

}  // namespace shrimp::shrimpfile

#endif  // INCLUDE_SHRIMP_FILE_HPP
