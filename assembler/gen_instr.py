import os
import sys

from instr import*

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write(
        "#ifndef ASSEMBLER_INSTR_GEN_HPP\n"
        "#define ASSEMBLER_INSTR_GEN_HPP\n\n"

        "#include <array>\n\n"

        "#include <shrimp/common/types.hpp>\n"
        "#include <shrimp/common/instr_opcode.gen.hpp>\n\n"

        "namespace shrimp {\n"
        "namespace assembler {\n\n"

        "template<InstrOpcode op>\n"
        "class Instr;\n\n"

        "struct InterfaceInstr {\n"
            "// Get instruction size in bytes\n"
            "virtual size_t getByteSize() const noexcept = 0;\n\n"

            "// Get ptr to buff with instruction binary code\n"
            "virtual const Byte *getBinCode() const noexcept = 0;\n"

            "virtual ~InterfaceInstr() = default;"
        "};\n\n"

        "struct InterfaceJump : public InterfaceInstr {\n"
            "// Set instruction target offset\n"
            "virtual void setOffset(ByteOffset offset) noexcept = 0;\n"
        "};\n\n"
    )

def write_instr_spec(out: TextIOWrapper, instr: Instr) :
    base = "InterfaceJump" if instr.is_jump else "InterfaceInstr"

    out.write("template<>\n")
    out.write("class Instr<%s> final : public %s {\n" % (instr.get_opcode_name(), base))

    out.write("public:\n")
    out.write("size_t getByteSize() const noexcept override { return %d; }\n" % INSTR_SIZES[instr.size])
    out.write("const Byte *getBinCode() const noexcept override { return reinterpret_cast<const Byte*>(&bin_code_); }\n\n")

    if instr.is_jump :
        jump_offset = instr.fields["jump_offset"]
        bit_size = jump_offset.get_bit_size()

        out.write("void setOffset(ByteOffset offset) noexcept override {\n")
        out.write("offset &= (uint64_t{ 1 } << %d) - 1;\n" % bit_size)
        out.write("bin_code_ |= offset << %d;\n" % jump_offset.lo)
        out.write("}\n\n")

    out.write("Instr(")

    first = True
    for field_name in instr.fields.keys() :
        if not first :
            out.write(", ")
        first = False

        out.write("uint64_t %s" % field_name)

    out.write(") {\n")

    for field_name, field in instr.fields.items() :
        out.write("// Write %s\n" % field_name)
        out.write("%s &= (uint64_t{ 1 } << %d) - 1;\n" % (field_name, field.get_bit_size()))
        out.write("bin_code_ |= %s << %d;\n\n" % (field_name, field.lo))

    out.write("}\n\n")
    out.write("private:\n")
    out.write("%s bin_code_ = static_cast<Byte>(%s);\n" % (instr.size, instr.get_opcode_name()))
    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write(
        "} // namespace assembler\n"
        "} // namespace shrimp\n\n"

        "#endif // ASSEMBLER_INSTR_GEN_HPP\n\n"
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
