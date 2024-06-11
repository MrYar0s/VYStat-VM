#ifndef INCLUDE_SHRIMP_ASSEMBLER_HPP
#define INCLUDE_SHRIMP_ASSEMBLER_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>
#include <charconv>
#include <algorithm>

#include <shrimp/common/types.hpp>
#include <shrimp/common/instr_opcode.gen.hpp>

#include <shrimp/assembler/instr.gen.hpp>
#include <shrimp/assembler/lexer.hpp>

#include <shrimp/shrimpfile.hpp>

namespace shrimp::assembler {

class Error : public std::runtime_error {
public:
    Error(std::string descr, size_t lineno)
        : std::runtime_error("Assembler error at line " + std::to_string(lineno) + " :" + descr)
    {
    }
};

class Assembler final {
    class JumpInfo final {
    public:
        JumpInfo(InterfaceJump *instr_ptr, ByteOffset instr_offset, std::string dst_name, int lineno)
            : instr_ptr_(instr_ptr), instr_offset_(instr_offset), dst_name_(std::move(dst_name)), lineno_(lineno)
        {
        }

        auto *getInstrPtr() const
        {
            return instr_ptr_;
        }
        auto getInstrOffset() const
        {
            return instr_offset_;
        }
        std::string getDstName() const
        {
            return dst_name_;
        }
        auto getLineno() const
        {
            return lineno_;
        }

    private:
        InterfaceJump *instr_ptr_ = nullptr;
        ByteOffset instr_offset_ = 0;
        std::string dst_name_ = "";
        int lineno_ = 0;
    };

    class FieldInfo final {
    public:
        FieldInfo(std::string name, size_t size, size_t offset, uint8_t is_ref)
            : name_(name), size_(size), offset_(offset), is_ref_(is_ref)
        {
        }

        auto isRef() const noexcept
        {
            return is_ref_;
        }

        auto getSize() const noexcept
        {
            return size_;
        }

        auto getOffset() const noexcept
        {
            return offset_;
        }

        const auto &getName() const noexcept
        {
            return name_;
        }

    private:
        // Field name
        std::string name_ = "";

        // Field type size
        size_t size_ = 0;

        // Field offset
        size_t offset_ = 0;

        // Field is_ref
        uint8_t is_ref_ = 0;
    };

    class ClassInfo final {
    public:
        ClassInfo(std::string name, size_t size) : name_(name), size_(size) {}

        const auto &name() const noexcept
        {
            return name_;
        }

        auto &fields() noexcept
        {
            return fields_;
        }

        const auto &fields() const noexcept
        {
            return fields_;
        }

        auto size() const noexcept
        {
            return size_;
        }

    private:
        // Class name
        std::string name_ = "";

        // Parsed fields
        std::vector<FieldInfo> fields_ {};

        // Class size
        size_t size_ = fields_.size();
    };

    class FuncInfo final {
    public:
        FuncInfo(ByteOffset offset, std::string name, std::vector<std::string> args)
            : offset_(offset), name_(name), args_(args)
        {
        }

        auto getArgs() const noexcept
        {
            return args_;
        }

        auto &instrs() noexcept
        {
            return instrs_;
        }
        auto &jumps() noexcept
        {
            return jumps_;
        }
        auto &labels() noexcept
        {
            return labels_;
        }

        const auto &name() const noexcept
        {
            return name_;
        }

        auto offset() const noexcept
        {
            return offset_;
        }

    private:
        // Function offset
        ByteOffset offset_ = 0;

        // Function name
        std::string name_ = "";

        // Function arguments aliases
        std::vector<std::string> args_ {};

        // Parsed instructions
        std::vector<std::unique_ptr<InterfaceInstr>> instrs_ {};
        // Parsed jumps
        std::vector<JumpInfo> jumps_ {};
        // Parsed labels: [Label name -> offset]
        std::unordered_map<std::string, ByteOffset> labels_ {};
    };

    // Try to get next lexem. Throw Error if failed
    void expectLexem()
    {
        if (lexer_.yylex() != Lexer::LEXING_OK) {
            throw Error("Failed to get next lexem", lexer_.lineno());
        }
    }

    // Try to get next lexem. Throw Error if failed
    void expectLexem(Lexer::LexemType lexem_type)
    {
        if (lexer_.yylex() != Lexer::LEXING_OK || lexer_.currLexemType() != lexem_type) {
            throw Error("Failed to get next lexem", lexer_.lineno());
        }
    }

    // Throw Error if condition is not true
    void assertParseError(bool cond)
    {
        if (!cond) {
            throw Error("Parse error", lexer_.lineno());
        }
    }

    // Parse expected register
    R8Id parseReg()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        R8Id reg_id = 0;
        const char *reg_name = lexer_.YYText();

        const auto &args = curr_func_->getArgs();
        bool found_arg = std::any_of(args.cbegin(), args.cend(), [&](const std::string &arg) {
            --reg_id;
            return std::strcmp(arg.c_str(), reg_name) == 0;
        });

        if (found_arg) {
            return reg_id;
        }

        assertParseError(std::toupper(lexer_.YYText()[0]) == 'R');

        const char *reg_id_ptr = lexer_.YYText() + 1;
        const char *reg_id_end = lexer_.YYText() + lexer_.YYLeng();

        auto [end, ec] = std::from_chars(reg_id_ptr, reg_id_end, reg_id);

        assertParseError(ec == std::errc());
        assertParseError(reg_id_end == end);

        return reg_id;
    }

    // Parse expected int64_t immediate
    int64_t parseImmI()
    {
        expectLexem(Lexer::LexemType::NUMBER);

        int64_t immi = 0;
        const char *immi_end = lexer_.YYText() + lexer_.YYLeng();

        auto [end, ec] = std::from_chars(lexer_.YYText(), immi_end, immi);

        assertParseError(ec == std::errc());
        assertParseError(immi_end == end);

        return immi;
    }

    // Parse expected floating point value
    double parseFP()
    {
        expectLexem(Lexer::LexemType::NUMBER);

        double value = 0;
        const char *value_end = lexer_.YYText() + lexer_.YYLeng();

        auto [end, ec] = std::from_chars(lexer_.YYText(), value_end, value);

        assertParseError(ec == std::errc());
        assertParseError(value_end == end);

        return value;
    }

    // Parse expected float immediate
    uint32_t parseImmF()
    {
        float value = parseFP();
        return std::bit_cast<uint32_t>(value);
    }

    // Parse expected double immediate
    uint64_t parseImmD()
    {
        double value = parseFP();
        return std::bit_cast<uint64_t>(value);
    }

    ClassId parseDefinedClassId()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        auto it = class_name_to_id_.find(lexer_.YYText());
        assertParseError(it != class_name_to_id_.end());

        return it->second;
    }

    FieldId parseClassFieldId(ClassId class_id)
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        const auto &class_fields = classes_[class_id].fields();

        auto field_it = std::find_if(class_fields.cbegin(), class_fields.cend(),
                                     [&](const FieldInfo &info) { return info.getName() == lexer_.YYText(); });

        assertParseError(field_it != class_fields.cend());
        return field_it - class_fields.cbegin();
    }

    // Add lexed label to current function labels
    void addLexedLabel()
    {
        assert(lexer_.currLexemType() == Lexer::LexemType::LABEL);

        std::string label_name = lexer_.YYText();
        // skip colon
        label_name.pop_back();

        auto [_, is_inserted] = curr_func_->labels().insert({label_name, curr_offset_});

        if (!is_inserted) {
            throw Error("Lable redeclaration: " + label_name, lexer_.lineno());
        }
    }

    StrId parseStrId(const std::string &str)
    {
        auto it = strings_.find(str);
        assertParseError(it != strings_.end());
        return it->second;
    }

    std::string parseString()
    {
        expectLexem(Lexer::LexemType::STRING);

        std::string str = lexer_.YYText();

        str = str.substr(1, str.size() - 2);
        strings_.insert({str, strings_.size()});

        return str;
    }

    // Parse jump destination label name
    std::string parseJumpDst()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);
        return lexer_.YYText();
    }

    IntrinsicCode parseIntrinsicName()
    {
        static std::unordered_map<std::string, IntrinsicCode> intrinsics = {{"PRINT.I32", IntrinsicCode::PRINT_I32},
                                                                            {"PRINT.F", IntrinsicCode::PRINT_F},
                                                                            {"PRINT.STR", IntrinsicCode::PRINT_STR},
                                                                            {"SCAN.I32", IntrinsicCode::SCAN_I32},
                                                                            {"SCAN.F", IntrinsicCode::SCAN_F},
                                                                            {"SIN", IntrinsicCode::SIN},
                                                                            {"COS", IntrinsicCode::COS},
                                                                            {"SQRT", IntrinsicCode::SQRT},
                                                                            {"CONCAT", IntrinsicCode::CONCAT},
                                                                            {"SUBSTR", IntrinsicCode::SUBSTR}};

        expectLexem(Lexer::LexemType::IDENTIFIER);

        std::string intrinsic_name = lexer_.YYText();
        std::transform(intrinsic_name.begin(), intrinsic_name.end(), intrinsic_name.begin(), ::toupper);

        auto it = intrinsics.find(intrinsic_name);
        assertParseError(it != intrinsics.end());

        return it->second;
    }

    std::array<R8Id, 4> parseIntrinsicArgs(IntrinsicCode code)
    {
        switch (code) {
            case IntrinsicCode::PRINT_I32:
            case IntrinsicCode::PRINT_F:
            case IntrinsicCode::PRINT_STR:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::SCAN_I32:
            case IntrinsicCode::SCAN_F:
                return {0, 0, 0, 0};

            case IntrinsicCode::SIN:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::COS:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::SQRT:
                expectLexem(Lexer::LexemType::COMMA);
                return {parseReg(), 0, 0, 0};

            case IntrinsicCode::CONCAT: {
                expectLexem(Lexer::LexemType::COMMA);
                auto reg1 = parseReg();
                expectLexem(Lexer::LexemType::COMMA);
                auto reg2 = parseReg();
                return {reg1, reg2, 0, 0};
            }

            case IntrinsicCode::SUBSTR: {
                expectLexem(Lexer::LexemType::COMMA);
                auto reg1 = parseReg();
                expectLexem(Lexer::LexemType::COMMA);
                auto reg2 = parseReg();
                return {reg1, reg2, 0, 0};
            }

            default:
                std::abort();
                // assert(0);
        }
    }

    FuncId parseCallFuncId()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        auto it = func_name_to_id_.find(lexer_.YYText());
        assertParseError(it != func_name_to_id_.end());

        return it->second;
    }

    void parseFuncDecl()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        std::string func_name = lexer_.YYText();

        func_name_to_id_.insert({func_name, funcs_.size()});

        expectLexem(Lexer::LexemType::LEFT_ROUND_BRACE);

        std::vector<std::string> args {};

        expectLexem();

        if (lexer_.currLexemType() == Lexer::LexemType::IDENTIFIER) {
            args.push_back(lexer_.YYText());

            while (expectLexem(), lexer_.currLexemType() == Lexer::LexemType::COMMA) {
                expectLexem(Lexer::LexemType::IDENTIFIER);
                args.push_back(lexer_.YYText());
            }
        }

        assertParseError(lexer_.currLexemType() == Lexer::LexemType::RIGHT_ROUND_BRACE);

        static constexpr size_t MAX_FUNC_ARGS_NUMBER = 4;
        assertParseError(args.size() <= MAX_FUNC_ARGS_NUMBER);

        funcs_.emplace_back(curr_offset_, func_name, args);
        curr_func_ = &funcs_.back();
    }

    // Generated
    int parseFuncBody();

    // Generated
    template <InstrOpcode>
    void parseInstr();

    void firstPass()
    {
        int lexing_status = lexer_.yylex();

        while (lexing_status == Lexer::LEXING_OK) {
            // Parse next function or class
            assertParseError(lexer_.currLexemType() == Lexer::LexemType::FUNC ||
                             lexer_.currLexemType() == Lexer::LexemType::CLASS);
            if (lexer_.currLexemType() == Lexer::LexemType::FUNC) {
                parseFuncDecl();
                lexing_status = parseFuncBody();
            } else if (lexer_.currLexemType() == Lexer::LexemType::CLASS) {
                parseClass();
            } else {
                assert(0);
            }
        }
    }

    uint8_t isRefType(const std::string &str)
    {
        if (str == "i32") {
            return 0U;
        } else if (str == "f") {
            return 0U;
        } else if (auto it = class_name_to_id_.find(str); it != class_name_to_id_.end()) {
            return 1U;
        } else {
            assertParseError(0);
            return 0U;
        }
    }

    size_t typeNameToSize(const std::string &str)
    {
        if (str == "i32") {
            return 8U;
        } else if (str == "f") {
            return 8U;
        } else if (auto it = class_name_to_id_.find(str); it != class_name_to_id_.end()) {
            return 8U;
        } else {
            assertParseError(0);
            return 0U;
        }
    }

    void parseClass()
    {
        expectLexem(Lexer::LexemType::IDENTIFIER);

        std::string class_name = lexer_.YYText();

        std::vector<FieldInfo> fields {};

        size_t class_size = 0;
        size_t counter = 0;

        while (lexer_.yylex() == Lexer::LEXING_OK) {
            if (lexer_.currLexemType() == Lexer::LexemType::FUNC || lexer_.currLexemType() == Lexer::LexemType::CLASS) {
                break;
            }

            std::string type = lexer_.YYText();

            expectLexem(Lexer::LexemType::IDENTIFIER);

            std::string field_name = lexer_.YYText();

            auto type_size = typeNameToSize(type);

            fields.emplace_back(field_name, type_size, counter, isRefType(type));

            counter++;

            class_size += type_size;
        }

        class_name_to_id_.insert({class_name, classes_.size()});

        classes_.emplace_back(class_name, class_size);
        classes_.back().fields() = std::move(fields);
    }

    void resloveJumps()
    {
        for (auto &&func : funcs_) {
            for (auto &&jump : func.jumps()) {
                auto lable_name = jump.getDstName();
                auto label_it = func.labels().find(lable_name.c_str());
                if (label_it == func.labels().end()) {
                    throw Error("Unresolved lable" + lable_name, jump.getLineno());
                }

                auto label_offset = label_it->second;

                jump.getInstrPtr()->setOffset(label_offset - jump.getInstrOffset());
            }
        }
    }

    void write(shrimp::shrimpfile::File &out)
    {
        writeCode(out);
        writeStrings(out);
        writeFunctions(out);
        writeClasses(out);
        out.fillHeaders();
    }

    void writeClasses(shrimp::shrimpfile::File &out)
    {
        for (auto &&klass : classes_) {
            writeClass(out, klass);
        }
    }

    void writeFunctions(shrimp::shrimpfile::File &out)
    {
        for (auto &&func : funcs_) {
            writeFunction(out, func);
        }
    }

    void writeStrings(shrimp::shrimpfile::File &out)
    {
        for (auto [str, str_id] : strings_) {
            writeString(out, str, str_id);
        }
    }

    void writeCode(shrimp::shrimpfile::File &out)
    {
        for (auto &&func : funcs_) {
            for (auto &&instr_ptr : func.instrs()) {
                const char *bin_code = reinterpret_cast<const char *>(instr_ptr->getBinCode());
                writeBytes(out, bin_code, instr_ptr->getByteSize());
            }
        }
    }

    void writeBytes(shrimp::shrimpfile::File &out, const char *bin_code, size_t size)
    {
        out.writeBytes(bin_code, size);
    }

    void writeClass(shrimp::shrimpfile::File &out, const ClassInfo &klass)
    {
        shrimpfile::File::FileClass class_info;
        class_info.name = klass.name();
        class_info.name_size = klass.name().size();
        class_info.size = klass.size();
        class_info.id = class_name_to_id_[class_info.name];
        auto &fields = class_info.fields;
        uint32_t start_id = 0;
        for (const auto &klass_field : klass.fields()) {
            auto &name = klass_field.getName();
            fields.push_back(shrimpfile::File::FileField {start_id++, klass_field.isRef(), klass_field.getSize(),
                                                          klass_field.getOffset(), name.size(), name});
        }
        class_info.num_of_fields = class_info.fields.size();
        out.writeClass(class_info);
    }

    void writeString(shrimp::shrimpfile::File &out, const std::string &str, StrId id)
    {
        out.writeString(str, id);
    }

    void writeFunction(shrimp::shrimpfile::File &out, const FuncInfo &func)
    {
        shrimpfile::File::FileFunction file_func;
        file_func.name = func.name();
        file_func.name_size = file_func.name.size();
        file_func.id = func_name_to_id_[file_func.name];
        file_func.func_start = func.offset();
        file_func.num_of_args = func.getArgs().size();
        file_func.num_of_vregs = 256;
        out.writeFunction(file_func);
    }

public:
    void assemble(const std::string &in, const std::string &out)
    {
        shrimpfile::File of {in, out};

        std::ifstream in_stream {in};

        lexer_.switch_streams(&in_stream);

        firstPass();
        resloveJumps();
        write(of);

        of.serialize();
    }

private:
    Lexer lexer_;

    ByteOffset curr_offset_ = 0;

    std::unordered_map<std::string, FuncId> func_name_to_id_ {};

    std::unordered_map<std::string, StrId> strings_ {};

    std::unordered_map<std::string, ClassId> class_name_to_id_ {};

    std::vector<FuncInfo> funcs_ {};

    std::vector<ClassInfo> classes_ {};

    FuncInfo *curr_func_ = nullptr;
};

template <>
inline void Assembler::parseInstr<InstrOpcode::INTRINSIC>()
{
    auto &&instrs = curr_func_->instrs();

    auto intrinsic_code_enum = parseIntrinsicName();
    auto [arg0, arg1, arg2, arg3] = parseIntrinsicArgs(intrinsic_code_enum);
    auto intrinsic_code = static_cast<uint8_t>(intrinsic_code_enum);

    instrs.push_back(std::make_unique<Instr<InstrOpcode::INTRINSIC>>(intrinsic_code, arg0, arg1, arg2, arg3));

    curr_offset_ += instrs.back()->getByteSize();
}

}  // namespace shrimp::assembler

#include <shrimp/assembler/parser.gen.hpp>

#endif  // INCLUDE_SHRIMP_ASSEMBLER_HPP
