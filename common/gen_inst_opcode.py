import os
import sys
import yaml

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write("#ifndef COMMON_INST_OPCODE_GEN_HPP\n")
    out.write("#define COMMON_INST_OPCODE_GEN_HPP\n\n")

    out.write("#include <cstdint>\n\n")

    out.write("namespace shrimp {\n\n")

def write_enum_open(out: TextIOWrapper) :
    out.write("enum class InstOpcode : uint8_t {\n")

def write_enum_value(out: TextIOWrapper, name: str, opcode: int) :
    out.write("%s = %d,\n" % (name.replace('.', '_'), opcode))

def write_enum_close(out: TextIOWrapper) :
    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write("} // namespace shrimp\n\n")

    out.write("#endif // COMMON_INST_OPCODE_GEN_HPP\n\n")

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    with open(IN_NAME, 'r') as file :
        insts = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    write_enum_open(out)
    for name, inst in insts.items() :
        write_enum_value(out, name, inst["opcode"])

    write_enum_close(out)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
