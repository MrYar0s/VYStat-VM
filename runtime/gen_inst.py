import os
import sys
import yaml

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write("#ifndef RUNTIME_INST_GEN_HPP\n")
    out.write("#define RUNTIME_INST_GEN_HPP\n\n")

    out.write("#include <inst_opcode.gen.hpp>\n\n")

    out.write("namespace shrimp {\n\n")

def write_inst_primary_templ(out: TextIOWrapper) :
    out.write("template<InstOpcode op>\n")
    out.write("class Inst;\n\n")

def write_inst_spec(out: TextIOWrapper, name: str, fields: dict[str, list] | None) :
    out.write("template<>\n")
    out.write("class Inst<InstOpcode::%s> {\n" % name.replace('.', '_'))
    if fields == None :
        out.write("};\n\n")
        return

    out.write("uint64_t m_value = 0;\n\n")
    out.write("public:\n")

    for field_name, field_bits in fields.items() :
        # assert(list.__len__ == 2)
        # Skip opcode
        lo = field_bits[0] + 8
        hi = field_bits[1] + 8

        assert(type(lo) == int)
        assert(type(hi) == int)
        assert(lo <= hi)
        assert(hi < 64)

        right_shift = 64 - 1 - hi
        left_shift = lo + right_shift

        out.write("[[nodiscard]] uint64_t get%s() const noexcept {\n" % field_name.capitalize())
        out.write("return m_value << %d >> %d;\n" % (right_shift, left_shift))
        out.write("}\n\n")

    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write("} // namespace shrimp\n\n")

    out.write("#endif // RUNTIME_INST_GEN_HPP\n\n")

if __name__ == "__main__" :
    IN_NAME = sys.argv[0]
    OUT_NAME = sys.argv[1]

    with open(IN_NAME, 'r') as file :
        insts = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    write_inst_primary_templ(out)
    for name, inst in insts.items() :
        write_inst_spec(out, name, inst.get("fields"))

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
