import os
import sys
import yaml

from io import TextIOWrapper

DWORD_BIT_SIZE = 64

def fix_instr_name(instr_name: str) :
    return instr_name.replace('.', '_')

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
    out.write("#ifndef RUNTIME_INSTR_GEN_HPP\n")
    out.write("#define RUNTIME_INSTR_GEN_HPP\n\n")

    out.write("#include <instr_opcode.gen.hpp>\n\n")

    out.write("namespace shrimp {\n\n")

def write_instr_primary_templ(out: TextIOWrapper) :
    out.write("template<InstrOpcode op>\n")
    out.write("class Instr;\n\n")

def write_instr_bin_code(out: TextIOWrapper, size: int) :
    out.write("private:\n")
    out.write("std::array<DWord, %d> bin_code_ {};\n" % size)

def write_instr_spec(out: TextIOWrapper, name: str, instr: dict) :
    fields = instr.get("fields")
    size = instr.get("size", 1)

    fixed_name = fix_instr_name(name)

    out.write("template<>\n")
    out.write("struct Instr<InstrOpcode::%s> {\n" % fixed_name)
    out.write("[[nodiscard]] static size_t getByteSize() const noexcept {\n")
    out.write("return %d;\n" % (size * 8))
    out.write("}\n")

    if fields == None :
        out.write("};\n\n")
        return

    out.write(
        "\n"
        "Instr(const Byte *pc) {\n"
        "std::memcpy(bin_code_.data(), pc, getByteSize());\n"
        "}\n\n"
    )

    for field_name, field_bits in fields.items() :
        [field_dword_idx, dword_lo, dword_hi, field_bit_len] = get_field_pos(field_name, field_bits, size)

        right_shift = DWORD_BIT_SIZE - dword_hi - 1
        left_shift = dword_lo + right_shift

        out.write("[[nodiscard]] uint64_t get%s() const noexcept {\n" % field_name.capitalize())
        out.write("return bin_code_[%d]" % field_dword_idx)
        if right_shift != 0 :
            out.write("<< %d" % right_shift)
        if left_shift != 0 :
            out.write(">> %d" % left_shift)
        out.write(";\n")
        out.write("}\n\n")

    write_instr_bin_code(out, size)
    out.write("};\n\n")

def write_file_close(out: TextIOWrapper) :
    out.write("} // namespace shrimp\n\n")

    out.write("#endif // RUNTIME_INSTR_GEN_HPP\n\n")

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    with open(IN_NAME, 'r') as file :
        instrs = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    write_instr_primary_templ(out)
    for name, instr in instrs.items() :
        write_instr_spec(out, name, instr)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
