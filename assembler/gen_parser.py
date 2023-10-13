import os
import sys
import yaml

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write("#ifndef PARSER_GEN_HPP\n")
    out.write("#define PARSER_GEN_HPP\n\n")

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
                out.write("InstWordNumber jump_offset = 0;")
                out.write("parseJumpDst();")
            case "intrinsic_code" :
                out.write("uint64_t intrinsic_code = parseIntrinsicName();")
            case "opt_reg_arg1" :
                parse_opt = True
                out.write("uint64_t opt_reg_arg1 = parseOptRegArg();")
                out.write("uint64_t prev_reg_arg = opt_reg_arg1;")
            case "opt_reg_arg2" | "opt_reg_arg3" | "opt_reg_arg4" :
                out.write("uint64_t %s = (prev_reg_arg == NO_OPT_REG_ARG) ? NO_OPT_REG_ARG :\
                          parseOptRegArg();" % field_name)
                out.write("prev_reg_arg = %s;" % field_name)

    out.write("\n")
    out.write("m_insts.push_back(std::make_unique<Inst<InstOpcode::%s>>(" % fixed_name)

    first = True
    for field_name in fields.keys() :
        if not first :
            out.write(", ")
        first = False
        out.write("%s" % field_name)

    out.write("));\n\n")
    out.write("m_curr_inst_word_number += m_insts.back()->getWordSize();\n")
    out.write("}\n\n")

def write_first_pass(out: TextIOWrapper, insts: dict) :
    out.write(
        "void Assembler::first_pass()\n"
        "{\n"
            "int lexing_status = Lexer::LEXING_ERROR_CODE;\n\n"

            "while ((lexing_status = m_lexer.yylex()) == Lexer::LEXING_OK) {\n"
                "if (m_lexer.currLexemType() == Lexer::LexemType::NEW_LINE) { continue; }\n"
                "assertParseError(m_lexer.currLexemType() == Lexer::LexemType::IDENTIFIER);\n\n"

                "std::string id = m_lexer.YYText();\n\n"
    )

    for inst_name in insts.keys() :
        fixed_name = inst_name.replace('.', '_')

        out.write("if (std::strcmp(\"%s\", id.c_str()) == 0) {\n" % inst_name)
        out.write("parseInst<InstOpcode::%s>();\n" % fixed_name)
        out.write("continue;")
        out.write("}")

    out.write(
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

