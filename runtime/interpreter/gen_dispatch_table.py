import os
import sys
import yaml

from io import TextIOWrapper

def fix_instr_name(instr_name: str) :
    return instr_name.replace('.', '_')

def instr_name_to_camel(instr_name: str) :
    return "".join(x.capitalize() for x in instr_name.lower().split("."))

def write_file_open(out: TextIOWrapper) :
    out.write(
        "#ifndef DISPATCH_TABLE_GEN_HPP\n"
        "#define DISPATCH_TABLE_GEN_HPP\n\n"

        "#include <array>\n\n"

        "#include <shrimp/common/types.hpp>\n"
        "#include <shrimp/common/instr_opcode.gen.hpp>\n"
        "#include <shrimp/runtime/interpreter.hpp>\n\n"

        "namespace shrimp::runtime::interpreter {\n\n"

        "namespace {\n\n"
    )

def write_handlers_decls(out: TextIOWrapper, instrs: dict) :
    for instr_name in instrs.keys() :
        out.write("int handle%s(const Byte *pc, Frame *frame);" % instr_name_to_camel(instr_name))

def write_dispatch_table(out: TextIOWrapper, instrs: dict) :
    sorted_instrs = dict(sorted(instrs.items(), key=lambda item: item[1]["opcode"]))

    DISPATCH_LEN = 256

    out.write("using HandlerPtr = int (*)(const Byte *pc, Frame *frame);\n\n")

    out.write("static constexpr size_t DISPATCH_LEN = %d;\n\n" % DISPATCH_LEN)

    # 0 is invalid opcode
    out.write("static constexpr std::array<HandlerPtr, DISPATCH_LEN> dispatch_table {\n")
    out.write("nullptr")

    curr_opcode = 1

    for instr_name, instr_descr in instrs.items() :
        next_instr_opcode = instr_descr["opcode"]

        while curr_opcode != next_instr_opcode :
            out.write(", nullptr")
            curr_opcode = curr_opcode + 1

        out.write(", handle%s" % instr_name_to_camel(instr_name))
        curr_opcode = curr_opcode + 1

    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write(
        "}  // namespace\n\n"

        "}  // namespace shrimp::runtime::interpreter\n\n"

        "#endif  // DISPATCH_TABLE_GEN_HPP\n"
    )

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    with open(IN_NAME, 'r') as file :
        instrs = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)
    write_handlers_decls(out, instrs)
    write_dispatch_table(out, instrs)
    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
