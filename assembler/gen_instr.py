import os
import sys
import yaml

from io import TextIOWrapper

def fix_instr_name(instr_name: str) :
    return instr_name.replace('.', '_')

DWORD_BIT_SIZE = 64

def get_field_pos(field_name: str, field: list, instr_dword_size: int) -> list :
    lo = field[0]
    hi = field[1]

    assert(type(lo) == int)
    assert(type(hi) == int)
    assert(lo <= hi)
    assert(hi < DWORD_BIT_SIZE * instr_dword_size)

    field_dword_idx = int(lo / DWORD_BIT_SIZE)
    dword_lo = lo - field_dword_idx * DWORD_BIT_SIZE
    dword_hi = hi - field_dword_idx * DWORD_BIT_SIZE

    field_bit_len = hi - lo + 1

    return [field_dword_idx, dword_lo, dword_hi, field_bit_len]

def write_file_open(out: TextIOWrapper) :
    out.write(
        "#ifndef ASSEMBLER_INSTR_GEN_HPP\n"
        "#define ASSEMBLER_INSTR_GEN_HPP\n\n"

        "#include <array>\n\n"

        "#include <shrimp/common/instr_opcode.gen.hpp>\n\n"

        "namespace shrimp {\n"
        "namespace assembler {\n\n"
    )

def write_instr_primary_templ(out: TextIOWrapper) :
    out.write(
        "template<InstrOpcode op>\n"
        "class Instr;\n\n"
    )

def write_instr_interface(out: TextIOWrapper) :
    out.write(
        "struct InterfaceInstr {\n"
            "// Get instruction size in words.\n"
            "// Note: Some instructions are several words long\n"
            "virtual std::size_t getByteSize() const noexcept = 0;\n\n"

            "// Get ptr to buff with instruction binary code\n"
            "virtual const DWord *getBinCode() const noexcept = 0;\n"
        "};\n\n"
    )

def write_jump_interface(out: TextIOWrapper) :
    out.write(
        "struct InterfaceJump : public InterfaceInstr {\n"
            "// Set instruction target in offset in ByteOffset.\n"
            "virtual void setOffset(ByteOffset offset) noexcept = 0;\n"
        "};\n\n"
    )

def write_instr_bin_code(out: TextIOWrapper, size: int, fixed_name: str) :
    out.write("private:\n")
    out.write("std::array<DWord, %d> bin_code_ = { static_cast<DWord>(InstrOpcode::%s) };\n" % (size, fixed_name))

def write_instr_spec(out: TextIOWrapper, name: str, instr: dict) :
    fields = instr.get("fields")
    size = instr.get("size", 1)

    is_jump = instr.get("is_jump", False)
    base = "InterfaceJump" if is_jump else "InterfaceInstr"

    fixed_name = fix_instr_name(name)

    out.write("template<>\n")
    out.write("class Instr<InstrOpcode::%s> final : public %s {\n" % (fixed_name, base))

    out.write("public:\n")
    out.write("std::size_t getByteSize() const noexcept override { return %d; }\n" % (size * 8))
    out.write("const DWord *getBinCode() const noexcept override { return bin_code_.data(); }\n\n")

    if fields == None :
        out.write("Instr() = default;\n")
        write_instr_bin_code(out, size, fixed_name)
        out.write("};\n\n")
        return

    if is_jump :
        [dword_idx, lo, hi, bit_len] = get_field_pos("jump_offset", fields["jump_offset"], size)

        out.write("void setOffset(ByteOffset offset) noexcept override {\n")
        if bit_len != DWORD_BIT_SIZE :
            out.write("offset &= (uint64_t{ 1 } << %d) - 1;\n" % bit_len)
        out.write("bin_code_[%d] |= offset << %d;\n" % (dword_idx, lo))
        out.write("}\n\n")

    out.write("Instr(")

    first = True
    for field_name in fields.keys() :
        if not first :
            out.write(", ")
        first = False

        out.write("uint64_t %s" % field_name)

    out.write(") {\n")

    for field_name, field_bits in fields.items() :
        [dword_idx, lo, hi, bit_len] = get_field_pos(field_name, field_bits, size)

        out.write("// Write %s\n" % field_name)
        if bit_len != DWORD_BIT_SIZE :
            out.write("%s &= (uint64_t{ 1 } << %d) - 1;\n" % (field_name, bit_len))
        out.write("bin_code_[%d] |= %s << %d;\n\n" % (dword_idx, field_name, lo))

    out.write("}\n\n")
    write_instr_bin_code(out, size, fixed_name)
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

    with open(IN_NAME, 'r') as file :
        instrs = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)
    write_instr_primary_templ(out)
    write_instr_interface(out)
    write_jump_interface(out)

    for name, instr in instrs.items() :
        write_instr_spec(out, name, instr)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
