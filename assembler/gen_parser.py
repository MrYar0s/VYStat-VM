import os
import sys
import yaml

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write("#ifndef PARSER_GEN_HPP\n")
    out.write("#define PARSER_GEN_HPP\n\n")

    out.write("#include <algorithm>\n")
    out.write("#include <types.hpp>\n")
    out.write("#include <shrimp/common/inst_opcode.gen.hpp>\n\n")

    out.write("namespace shrimp {\n")
    out.write("namespace assembler {\n\n")

def write_inst_parser(out: TextIOWrapper, name: str, fields: dict[str, list] | None) :
    fixed_name = name.replace('.', '_')

    out.write("template<>\n")
    out.write("void Assembler::parseInst<InstOpcode::%s>() {\n" % fixed_name)

    if fields == None :
        out.write("m_insts.push_back(std::make_unique<Inst<InstOpcode::%s>>());\n" % fixed_name)
        out.write("}\n\n")
        return

    first = True
    for field_name in fields.keys() :
        if not first and field_name.find("opt_reg_arg") == -1 :
            out.write("parseComma();\n")
        first = False

        match field_name :
            case "rd" | "rs" | "rs1" | "rs2" :
                out.write("uint64_t %s = parseReg();\n" % field_name)
            case "imm_i32" | "imm_i64" :
                out.write("uint64_t %s = parseImmI();\n" % field_name)
            case "imm_f" | "imm_d" :
                out.write("double %s = parseImmD();\n" % field_name)
            case "jump_offset" :
                out.write("DWordOffset jump_offset = 0;")
                out.write("parseJumpDst();")
            case "intrinsic_code" :
                out.write("auto intrinsic_code_enum = parseIntrinsicName();")
                out.write("auto intrinsic_args = parseIntrinsicArgs(intrinsic_code_enum);")
                out.write("auto intrinsic_code = static_cast<uint8_t>(intrinsic_code_enum);")
            case "opt_reg_arg1" | "opt_reg_arg2" | "opt_reg_arg3" | "opt_reg_arg4" :
                out.write("R8Id %s = intrinsic_args[%c];" % (field_name, field_name[-1]))

            case _:
                raise RuntimeError("Unknown field")

    out.write("\n")
    out.write("m_insts.push_back(std::make_unique<Inst<InstOpcode::%s>>(" % fixed_name)

    first = True
    for field_name in fields.keys() :
        if not first :
            out.write(", ")
        first = False
        out.write("%s" % field_name)

    out.write("));\n\n")
    out.write("m_curr_dword_offset += m_insts.back()->getDWordSize();\n")
    out.write("}\n\n")

def write_first_pass(out: TextIOWrapper, insts: dict) :
    out.write(
        "void Assembler::first_pass()\n"
        "{\n"
        "static std::unordered_map<std::string, InstOpcode> insts = {"
    )

    first = True
    for inst_name in insts.keys() :
        if not first :
            out.write(", ")
        first = False

        fixed_name = inst_name.replace('.', '_')
        out.write("{\"%s\", InstOpcode::%s}" % (inst_name, fixed_name))

    out.write("};\n\n")

    out.write(
        "int lexing_status = Lexer::LEXING_ERROR_CODE;\n\n"

        "while ((lexing_status = m_lexer.yylex()) == Lexer::LEXING_OK) {\n"
            "assertParseError(m_lexer.currLexemType() == Lexer::LexemType::IDENTIFIER);\n\n"

            "std::string id = m_lexer.YYText();\n"
            "std::transform(id.begin(), id.end(), id.begin(), ::toupper);\n\n"

            "auto it = insts.find(id);\n"
            "assertParseError(it != insts.end());\n\n"

            "switch(it->second) {\n"
    )

    for inst_name in insts.keys() :
        fixed_name = inst_name.replace('.', '_')

        out.write("case InstOpcode::%s:\n" % fixed_name)
        out.write("parseInst<InstOpcode::%s>();\n" % fixed_name)
        out.write("break;\n")

    out.write(
        "default: assert(0);"

                "}"
            "}"
        "}"
    )

def write_file_close(out: TextIOWrapper) :
    out.write("} // namespace assembler\n")
    out.write("} // namespace shrimp\n\n")

    out.write("#endif // PARSER_GEN_HPP\n\n")

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    with open(IN_NAME, 'r') as file :
        insts = yaml.safe_load(file)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    for name, inst in insts.items() :
        write_inst_parser(out, name, inst.get("fields"))

    write_first_pass(out, insts)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)

