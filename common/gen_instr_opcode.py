import os
import sys

from isa import*

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write(
        "#ifndef COMMON_INSTR_OPCODE_GEN_HPP\n"
        "#define COMMON_INSTR_OPCODE_GEN_HPP\n\n"

        "#include <cstdint>\n\n"

        "namespace shrimp {\n\n"

        "enum class InstrOpcode : uint8_t {\n"
    )

def write_enum_value(out: TextIOWrapper, instr: Instr) :
    out.write("%s = %d,\n" % (instr.get_sneak_name(), instr.opcode))

def write_file_close(out: TextIOWrapper) :
    out.write(
        "};\n\n"

        "} // namespace shrimp\n\n"

        "#endif // COMMON_INSTR_OPCODE_GEN_HPP\n"
    )

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    instrs = load_instrs(IN_NAME)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    for instr in instrs :
        write_enum_value(out, instr)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
