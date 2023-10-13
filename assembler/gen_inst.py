import os
import sys
import yaml

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write("#ifndef ASSEMBLER_INST_GEN_HPP\n")
    out.write("#define ASSEMBLER_INST_GEN_HPP\n\n")

    out.write("#include <array>\n\n")

    out.write("#include <shrimp/common/inst_opcode.gen.hpp>\n\n")

    out.write("namespace shrimp {\n")
    out.write("namespace assembler {\n\n")

def write_inst_primary_templ(out: TextIOWrapper) :
    out.write("template<InstOpcode op>\n")
    out.write("class Inst;\n\n")

def write_inst_interface(out: TextIOWrapper) :
    out.write(
        "struct InterfaceInst {\n"
        "// Get instruction size in words.\n"
        "// Note: Some instructions are several words long\n"
        "virtual std::size_t getDWordSize() const noexcept = 0;\n"

        "// Get ptr to buff with instruction binary code\n"
        "virtual const DWord *getBinCode() const noexcept = 0;\n"
        "};\n\n"
    )

def write_jump_interface(out: TextIOWrapper) :
    out.write(
        "struct InterfaceJump : public InterfaceInst {\n"
        "// Set instruction target in offset in DWordOffset.\n"
        "virtual void setOffset(DWordOffset offset) noexcept = 0;\n"
        "};\n\n"
    )


def write_inst_spec(out: TextIOWrapper, name: str, inst: dict) :
    fields = inst.get("fields")
    is_jump = inst.get("is_jump", False)
    size = inst.get("size", 1)

    fixed_name = name.replace('.', '_')

    out.write("template<>\n")
    out.write("class Inst<InstOpcode::%s> final : public " % fixed_name)
    if is_jump :
        out.write("InterfaceJump {\n")
    else :
        out.write("InterfaceInst {\n")

    out.write("std::array<DWord, %d> m_bin_code = { static_cast<DWord>(InstOpcode::%s) };\n\n" % (size, fixed_name))

    out.write("public:\n")
    out.write("std::size_t getDWordSize() const noexcept override { return m_bin_code.size(); }\n")
    out.write("const DWord *getBinCode() const noexcept override { return m_bin_code.data(); }\n\n")

    if is_jump :
        jump_offset = fields["jump_offset"]

        lo = jump_offset[0]
        hi = jump_offset[1]

        jump_word = int(lo / 64)
        lo = lo - jump_word * 64
        hi = hi - jump_word * 64

        field_len = hi - lo + 1

        out.write("void setOffset(DWordOffset offset) noexcept override {\n")
        out.write("offset &= (uint64_t{ 1 } << %d) - 1;\n" % field_len)
        out.write("m_bin_code[%d] |= offset << %d;\n" % (jump_word, lo))
        out.write("}\n\n")

    if fields == None :
        out.write("Inst() = default;\n")
        out.write("};\n\n")
        return

    out.write("Inst(")

    first = True
    for field_name in fields.keys() :
        if not first :
            out.write(", ")
        first = False

        out.write("uint64_t %s" % field_name)

    out.write(") {\n")

    for field_name, field_bits in fields.items() :
        lo = field_bits[0]
        hi = field_bits[1]

        assert(type(lo) == int)
        assert(type(hi) == int)
        assert(lo <= hi)
        assert(hi < 64 * size)

        field_word = int(lo / 64)
        lo = lo - field_word * 64
        hi = hi - field_word * 64

        field_len = hi - lo + 1

        out.write("    // Write %s\n" % field_name)
        if field_len != 64 :
            out.write("%s &= (uint64_t{ 1 } << %d) - 1;\n" % (field_name, field_len))
        out.write("m_bin_code[%d] |= %s << %d;\n\n" % (field_word, field_name, lo))

    out.write("}\n")
    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write("} // namespace assembler\n")
    out.write("} // namespace shrimp\n\n")

    out.write("#endif // ASSEMBLER_INST_GEN_HPP\n\n")

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    with open(IN_NAME, 'r') as file :
        insts = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)
    write_inst_primary_templ(out)
    write_inst_interface(out)
    write_jump_interface(out)

    for name, inst in insts.items() :
        write_inst_spec(out, name, inst)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
