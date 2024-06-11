import os
import sys
import yaml

from io import TextIOWrapper

def fix_instr_name(instr_name: str) :
    return instr_name.replace('.', '_')

def instr_name_to_camel(instr_name: str) :
    return "".join(x.capitalize() for x in instr_name.lower().split("."))

def write_dispatch_table(out: TextIOWrapper, instrs: dict) :
    # 0 is invalid opcode
    out.write("static void *dispatch_table[] = {\n")
    out.write("&&handleInvalidOpcode")

    curr_opcode = 1

    for instr_name, instr_descr in instrs.items() :
        next_instr_opcode = instr_descr["opcode"]

        while curr_opcode != next_instr_opcode :
            out.write(", &&handleInvalidOpcode")
            curr_opcode = curr_opcode + 1

        if instr_descr.get("gen_handler", True) :
            out.write(", &&handle%s" % instr_name_to_camel(instr_name))
        else :
            out.write(", &&handleInvalidOpcode")

        curr_opcode = curr_opcode + 1

    out.write("};\n\n")

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    with open(IN_NAME, 'r') as file :
        instrs = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_dispatch_table(out, instrs)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
