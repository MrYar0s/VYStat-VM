import os
import sys

from instr import*

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write(
        "#ifndef RUNTIME_INSTR_GEN_HPP\n"
        "#define RUNTIME_INSTR_GEN_HPP\n\n"

        "#include <array>\n"
        "#include <cstring>\n\n"

        "#include <shrimp/common/instr_opcode.gen.hpp>\n\n"

        "namespace shrimp::runtime::interpreter {\n\n"

        "template<InstrOpcode op>\n"
        "struct Instr;\n\n"
    )

def write_instr_bin_code(out: TextIOWrapper, size: int) :
    out.write("private:\n")
    out.write("std::array<DWord, %d> bin_code_ {};\n" % size)

def write_instr_to_string(out: TextIOWrapper, instr: Instr) :
    if instr.name == "INTRINSIC" :
        out.write("std::string toString() const;\n\n")
        return

    out.write("[[nodiscard]] std::string toString() const {\n")
    out.write("std::stringstream out;\n\n")

    out.write("out << \"%s\"\n" % instr.name)

    need_comma = False
    for field_name, field in instr.fields.items() :
        camel_field_name = name_to_camel(field_name)

        if need_comma :
            out.write("<< \", \" <<")
        else :
            out.write("<< \" \" <<")

        need_comma = True

        match field_name :
            case "rd" | "rs" | "rs1" | "rs2" | "func_arg0" | "func_arg1" | "func_arg2" | "func_arg3" :
                out.write("\"R\" << get%s()" % camel_field_name)

            case "imm_i32" :
                out.write("bit::getValue<int32_t>(get%s())" % camel_field_name)
            case "imm_i64" | "jump_offset" | "func_id" | "str_id" :
                out.write("get%s()" % camel_field_name)
            case "imm_f" :
                out.write("bit::getValue<float>(get%s())" % camel_field_name)
            case "imm_d" :
                out.write("bit::getValue<double>(get%s())" % camel_field_name)

            case _:
                raise RuntimeError("Unknown field in instruction %s: %s" % (instr.name, field_name))

    out.write(
        ";\n"
        "return out.str();\n"
        "}\n\n"
    )

def write_instr_spec(out: TextIOWrapper, instr: Instr) :
    out.write("template<>\n")
    out.write("struct Instr<%s> {\n" % instr.get_opcode_name())
    out.write("[[nodiscard]] static size_t getByteSize() noexcept {\n")
    out.write("return %d;\n" % (INSTR_SIZES[instr.size]))
    out.write("}\n\n")

    write_instr_to_string(out, instr)

    out.write(
        "Instr(const Byte *pc) {\n"
        "std::memcpy(&bin_code_, pc, getByteSize());\n"
        "}\n\n"
    )

    for field_name, field in instr.fields.items() :
        right_shift = INSTR_BIT_SIZES[instr.size] - field.hi - 1
        left_shift = field.lo + right_shift

        out.write("[[nodiscard]] uint64_t get%s() const noexcept {\n" % name_to_camel(field_name))
        out.write("auto unsigned_value = bin_code_ << %d >> %d;\n" % (right_shift, left_shift))
        if field.is_signed :
            out.write("return bit::signExtend<uint64_t, %d>(unsigned_value);\n" % (field.get_bit_size() - 1))
        else :
            out.write("return unsigned_value;\n")

        out.write("}\n\n")

    out.write("private:\n")
    out.write("%s bin_code_ = 0;\n" % instr.size)
    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write(
        "} // namespace shrimp::runtime::interpreter\n\n"

        "#endif // RUNTIME_INSTR_GEN_HPP\n\n"
    )

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    instrs = load_instrs(IN_NAME)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    for instr in instrs :
        write_instr_spec(out, instr)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
