#include <cstdint>
#include <cstring>
#include <cstdio>
#include <filesystem>

#include <shrimp/shrimpfile.hpp>

static void ownRead(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret = fread(ptr, size, nmemb, stream);
    if (ret != nmemb) {
        std::abort();
    }
}
static void ownWrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t ret = fwrite(ptr, size, nmemb, stream);
    if (ret != nmemb) {
        std::abort();
    }
}

namespace shrimp::shrimpfile {

File::File([[maybe_unused]] const std::string &src_file_path, const std::string &bin_file_path)
    : bin_file_path_ {bin_file_path}
{
    std::filesystem::path o_path(bin_file_path);

    std::string bin_file_name = o_path.filename().string();

    std::memcpy(FileHeader.magic, "shrimp\0\0", MAGIC_SIZE);
    std::memcpy(FileHeader.bin_file_name, bin_file_name.data(), FILENAME_SIZE);
}

File::File(const std::string &file_name)
{
    // Open file
    FILE *file = fopen(file_name.data(), "rb");
    if (file == NULL) {
        std::cerr << "No such file" << std::endl;
        std::abort();
    }

    // Read FileHeader
    ownRead(FileHeader.magic, sizeof(uint8_t), MAGIC_SIZE, file);
    ownRead(FileHeader.bin_file_name, sizeof(uint8_t), FILENAME_SIZE, file);
    ownRead(&FileHeader.headers_num, sizeof(FileHeader.headers_num), 1, file);
    ownRead(&FileHeader.file_header_size, sizeof(FileHeader.file_header_size), 1, file);
    ownRead(FileHeader.headers, sizeof(SegmentsHeader), Headers::HEADERS_NUM, file);

    // Read FileClassHeader
    ownRead(&FileClassHeader.num, sizeof(FileClassHeader.num), 1, file);
    ownRead(&FileClassHeader.header_size, sizeof(FileClassHeader.header_size), 1, file);

    // Read FileStringHeader
    ownRead(&FileStringHeader.num, sizeof(FileStringHeader.num), 1, file);
    ownRead(&FileStringHeader.header_size, sizeof(FileStringHeader.header_size), 1, file);

    // Read FileFunctionHeader
    ownRead(&FileFuncHeader.num, sizeof(FileFuncHeader.num), 1, file);
    ownRead(&FileFuncHeader.header_size, sizeof(FileFuncHeader.header_size), 1, file);

    // Read Class Section
    std::vector<FileClass> classes(FileClassHeader.num);
    fseek(file, FileHeader.headers[CLASSES].offset_from_start, SEEK_SET);
    for (auto &class_info : classes) {
        ownRead(&class_info.id, sizeof(class_info.id), 1, file);
        ownRead(&class_info.size, sizeof(class_info.size), 1, file);
        ownRead(&class_info.name_size, sizeof(class_info.name_size), 1, file);
        std::vector<char> vec_of_str(class_info.name_size);
        ownRead(vec_of_str.data(), sizeof(char), vec_of_str.size(), file);
        class_info.name = std::string {vec_of_str.begin(), vec_of_str.end()};
        ownRead(&class_info.num_of_fields, sizeof(class_info.num_of_fields), 1, file);
        std::vector<FileField> fields(class_info.num_of_fields);
        for (auto &field : fields) {
            ownRead(&field.id, sizeof(field.id), 1, file);
            ownRead(&field.size, sizeof(field.size), 1, file);
            ownRead(&field.name_size, sizeof(field.name_size), 1, file);
            std::vector<char> vec_of_str_field(field.name_size);
            ownRead(vec_of_str_field.data(), sizeof(char), vec_of_str_field.size(), file);
            field.name = std::string {vec_of_str_field.begin(), vec_of_str_field.end()};
        }
        class_info.fields = std::move(fields);
    }
    Classes = classes;

    // Read Code Section
    std::vector<Byte> code(FileHeader.headers[CODE].size);
    fseek(file, FileHeader.headers[CODE].offset_from_start, SEEK_SET);
    ownRead(code.data(), sizeof(Byte), FileHeader.headers[CODE].size, file);
    Code = code;

    // Read String Section
    std::vector<FileString> strings(FileStringHeader.num);
    fseek(file, FileHeader.headers[LITERALS].offset_from_start, SEEK_SET);
    for (auto &string_info : strings) {
        ownRead(&string_info.id, sizeof(string_info.id), 1, file);
        ownRead(&string_info.str_size, sizeof(string_info.str_size), 1, file);
        std::vector<char> vec_of_str(string_info.str_size);
        ownRead(vec_of_str.data(), sizeof(char), vec_of_str.size(), file);
        string_info.str = std::string {vec_of_str.begin(), vec_of_str.end()};
    }
    Strings = strings;

    // Read Func Section
    std::vector<FileFunction> functions(FileFuncHeader.num);
    fseek(file, FileHeader.headers[FUNCTIONS].offset_from_start, SEEK_SET);
    for (auto &func_info : functions) {
        ownRead(&func_info.id, sizeof(func_info.id), 1, file);
        ownRead(&func_info.func_start, sizeof(func_info.func_start), 1, file);
        ownRead(&func_info.name_size, sizeof(func_info.name_size), 1, file);
        ownRead(&func_info.num_of_args, sizeof(func_info.num_of_args), 1, file);
        ownRead(&func_info.num_of_vregs, sizeof(func_info.num_of_vregs), 1, file);
        std::vector<char> vec_of_str(func_info.name_size);
        ownRead(vec_of_str.data(), sizeof(char), vec_of_str.size(), file);
        func_info.name = std::string {vec_of_str.begin(), vec_of_str.end()};
    }
    Functions = functions;

    fclose(file);
}

void File::fillHeaders()
{
    fillClassesHeader();
    fillCodeHeader();
    fillStringsHeader();
    fillFunctionsHeader();
}

void File::fillCodeHeader()
{
    auto &codeHeader = FileHeader.headers[CODE];
    codeHeader.offset_from_start = FileHeader.file_header_size + FileStringHeader.header_size +
                                   FileFuncHeader.header_size + FileClassHeader.header_size +
                                   FileHeader.headers[CLASSES].size;
    codeHeader.size = Code.size();
}

void File::fillStringsHeader()
{
    auto &stringHeader = FileHeader.headers[LITERALS];
    stringHeader.offset_from_start = FileHeader.file_header_size + FileStringHeader.header_size +
                                     FileHeader.headers[CODE].size + FileFuncHeader.header_size +
                                     FileHeader.headers[CLASSES].size + FileClassHeader.header_size;
    FileStringHeader.num = Strings.size();
    stringHeader.size = 0;
    for (auto &str : Strings) {
        stringHeader.size += sizeof(str.id) + sizeof(str.str_size) + str.str.size();
    }
}

void File::fillFunctionsHeader()
{
    auto &funcHeader = FileHeader.headers[FUNCTIONS];
    funcHeader.offset_from_start = FileHeader.file_header_size + FileFuncHeader.header_size +
                                   FileHeader.headers[CODE].size + FileHeader.headers[LITERALS].size +
                                   FileHeader.headers[CLASSES].size + FileStringHeader.header_size +
                                   FileClassHeader.header_size;
    FileFuncHeader.num = Functions.size();
    funcHeader.size = 0;
    for (auto &func : Functions) {
        funcHeader.size += sizeof(func.id) + sizeof(func.func_start) + sizeof(func.name_size) +
                           sizeof(func.num_of_args) + sizeof(func.num_of_vregs) + func.name.size();
    }
}

void File::fillClassesHeader()
{
    auto &classesHeader = FileHeader.headers[CLASSES];
    classesHeader.offset_from_start = FileHeader.file_header_size + FileStringHeader.header_size +
                                      FileFuncHeader.header_size + FileClassHeader.header_size;
    FileClassHeader.num = Classes.size();
    classesHeader.size = 0;
    for (auto &klass : Classes) {
        classesHeader.size += sizeof(klass.id) + sizeof(klass.size) + sizeof(klass.name_size) + klass.name.size() +
                              sizeof(klass.num_of_fields);
        for (auto &field : klass.fields) {
            classesHeader.size += sizeof(field.id) + sizeof(field.size) + sizeof(field.name_size) + field.name.size();
        }
    }
}

void File::writeBytes(const char *bin_code, size_t size)
{
    Code.insert(Code.end(), bin_code, bin_code + size);
}

void File::writeString(const std::string &str, StrId str_id)
{
    Strings.push_back(FileString {str_id, str.size(), str});
}

void File::writeFunction(const FileFunction &func)
{
    Functions.push_back(func);
}

void File::writeClass(const FileClass &klass)
{
    Classes.push_back(klass);
}

void File::serialize()
{
    std::FILE *out = std::fopen(bin_file_path_.data(), "wb");
    serializeFileHeader(out);
    serializeHeaders(out);
    serializeClasses(out);
    serializeCode(out);
    serializeStrings(out);
    serializeFunctions(out);
    fclose(out);
}

void File::serializeFileHeader(std::FILE *out)
{
    ownWrite(FileHeader.magic, sizeof(uint8_t), MAGIC_SIZE, out);
    ownWrite(FileHeader.bin_file_name, sizeof(uint8_t), FILENAME_SIZE, out);
    ownWrite(&FileHeader.headers_num, sizeof(FileHeader.headers_num), 1, out);
    ownWrite(&FileHeader.file_header_size, sizeof(FileHeader.file_header_size), 1, out);
    ownWrite(FileHeader.headers, sizeof(SegmentsHeader), Headers::HEADERS_NUM, out);
}

void File::serializeHeaders(std::FILE *out)
{
    ownWrite(&FileClassHeader.num, sizeof(FileClassHeader.num), 1, out);
    ownWrite(&FileClassHeader.header_size, sizeof(FileClassHeader.header_size), 1, out);
    ownWrite(&FileStringHeader.num, sizeof(FileStringHeader.num), 1, out);
    ownWrite(&FileStringHeader.header_size, sizeof(FileStringHeader.header_size), 1, out);
    ownWrite(&FileFuncHeader.num, sizeof(FileFuncHeader.num), 1, out);
    ownWrite(&FileFuncHeader.header_size, sizeof(FileFuncHeader.header_size), 1, out);
}

void File::serializeCode(std::FILE *out)
{
    ownWrite(Code.data(), sizeof(Byte), Code.size(), out);
}

void File::serializeStrings(std::FILE *out)
{
    for (auto &str : Strings) {
        ownWrite(&str.id, sizeof(str.id), 1, out);
        ownWrite(&str.str_size, sizeof(str.str_size), 1, out);
        ownWrite(str.str.data(), sizeof(char), str.str_size, out);
    }
}

void File::serializeFunctions(std::FILE *out)
{
    for (auto &func : Functions) {
        ownWrite(&func.id, sizeof(func.id), 1, out);
        ownWrite(&func.func_start, sizeof(func.func_start), 1, out);
        ownWrite(&func.name_size, sizeof(func.name_size), 1, out);
        ownWrite(&func.num_of_args, sizeof(func.num_of_args), 1, out);
        ownWrite(&func.num_of_vregs, sizeof(func.num_of_vregs), 1, out);
        ownWrite(func.name.data(), sizeof(char), func.name.size(), out);
    }
}

void File::serializeClasses(std::FILE *out)
{
    for (auto &klass : Classes) {
        ownWrite(&klass.id, sizeof(klass.id), 1, out);
        ownWrite(&klass.size, sizeof(klass.size), 1, out);
        ownWrite(&klass.name_size, sizeof(klass.name_size), 1, out);
        ownWrite(klass.name.data(), sizeof(char), klass.name_size, out);
        ownWrite(&klass.num_of_fields, sizeof(klass.num_of_fields), 1, out);
        for (auto &field : klass.fields) {
            ownWrite(&field.id, sizeof(field.id), 1, out);
            ownWrite(&field.size, sizeof(field.size), 1, out);
            ownWrite(&field.name_size, sizeof(field.name_size), 1, out);
            ownWrite(field.name.data(), sizeof(char), field.name_size, out);
        }
    }
}

std::string File::dump()
{
    std::stringstream ss;
    dumpFileHeader(ss);
    dumpClassHeader(ss);
    dumpClasses(ss);
    dumpCodeHeader(ss);
    dumpCode(ss);
    dumpStringHeader(ss);
    dumpString(ss);
    dumpFunctionHeader(ss);
    dumpFunction(ss);
    return ss.str();
}

void File::dumpFileHeader(std::stringstream &ss)
{
    ss << "Magic: " << FileHeader.magic << std::endl;
    ss << "Binary File Name: " << FileHeader.bin_file_name << std::endl;
    ss << "Num of Headers: " << FileHeader.headers_num << std::endl;
    ss << "File Header Size: " << FileHeader.file_header_size << std::endl;
}

void File::dumpCodeHeader(std::stringstream &ss)
{
    auto &codeHeader = FileHeader.headers[CODE];
    ss << "Code Size: " << codeHeader.size << std::endl;
    ss << "Code Header Offset To Segment From Start: " << codeHeader.offset_from_start << std::endl;
}

void File::dumpCode(std::stringstream &ss)
{
    for (auto &line : Code) {
        ss << std::hex << line << std::dec << std::endl;
    }
}

void File::dumpStringHeader(std::stringstream &ss)
{
    auto &stringPreHeader = FileHeader.headers[LITERALS];
    ss << "String Pre Header Size: " << stringPreHeader.size << std::endl;
    ss << "String Pre Header Offset To Segment From Start: " << stringPreHeader.offset_from_start << std::endl;
    ss << "Num of strings: " << FileStringHeader.num << std::endl;
    ss << "String Header Size: " << FileStringHeader.header_size << std::endl;
}

void File::dumpString(std::stringstream &ss)
{
    for (auto &str : Strings) {
        ss << "String Id: " << static_cast<uint32_t>(str.id) << std::endl;
        ss << "String size: " << str.str_size << std::endl;
        ss << "String: " << str.str << std::endl;
    }
}

void File::dumpFunctionHeader(std::stringstream &ss)
{
    auto &funcPreHeader = FileHeader.headers[FUNCTIONS];
    ss << "Function Pre Header Size: " << funcPreHeader.size << std::endl;
    ss << "Function Pre Header Offset To Segment From Start: " << funcPreHeader.offset_from_start << std::endl;
    ss << "Num of functions: " << FileFuncHeader.num << std::endl;
    ss << "Function Header Size: " << FileFuncHeader.header_size << std::endl;
}

void File::dumpFunction(std::stringstream &ss)
{
    for (auto &func : Functions) {
        ss << "Func Id: " << static_cast<uint32_t>(func.id) << std::endl;
        ss << "Func offset: " << func.func_start << std::endl;
        ss << "Func name_size: " << func.name_size << std::endl;
        ss << "Func num_of_args: " << static_cast<uint32_t>(func.num_of_args) << std::endl;
        ss << "Func num_of_vregs: " << func.num_of_vregs << std::endl;
        ss << "Func name: " << func.name << std::endl;
    }
}

void File::dumpClassHeader(std::stringstream &ss)
{
    auto classPreHeader = FileHeader.headers[CLASSES];
    ss << "Class Pre Header Size: " << classPreHeader.size << std::endl;
    ss << "Class Pre Header Offset To Segment From Start: " << classPreHeader.offset_from_start << std::endl;
    ss << "Num of classes: " << FileClassHeader.num << std::endl;
    ss << "Class Header Size: " << FileClassHeader.header_size << std::endl;
}

void File::dumpClasses(std::stringstream &ss)
{
    for (auto &klass : Classes) {
        ss << "ClassId : " << klass.id << std::endl;
        ss << "Class size : " << klass.size << std::endl;
        ss << "Class name_size : " << klass.name_size << std::endl;
        ss << "Class name : " << klass.name << std::endl;
        ss << "Class num_of_fields : " << klass.num_of_fields << std::endl;
        ss << "## Start dump of Fields ##" << std::endl;
        for (auto &field : klass.fields) {
            ss << "FieldId : " << field.id << std::endl;
            ss << "Field size : " << field.size << std::endl;
            ss << "Field name_size : " << field.name_size << std::endl;
            ss << "Field name : " << field.name << std::endl;
        }
        ss << "## End dump of Fields ##" << std::endl;
    }
}

}  // namespace shrimp::shrimpfile