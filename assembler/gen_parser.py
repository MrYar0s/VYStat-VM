import os
import sys
import re

from isa import*

from io import TextIOWrapper

def write_file_open(out: TextIOWrapper) :
    out.write(
        "#ifndef PARSER_GEN_HPP\n"
        "#define PARSER_GEN_HPP\n\n"

        "#include <algorithm>\n"
        "#include <shrimp/common/types.hpp>\n"
        "#include <shrimp/common/instr_opcode.gen.hpp>\n\n"

        "namespace shrimp::assembler {\n\n"
    )

def write_instr_parser(out: TextIOWrapper, instr: Instr) :
    if not instr.gen_parser:
        return

    out.write("template<>\n")
    out.write("void Assembler::parseInstr<%s>() {\n" % instr.get_opcode_name())
    out.write("auto &&instrs = curr_func_->instrs();\n")

    need_comma = False
    for field_name in instr.fields.keys() :
        if need_comma :
            out.write("expectLexem(Lexer::LexemType::COMMA);")

        need_comma = True

        match field_name :
            case "rd" | "rs" | "rs1" | "rs2" | "func_arg0" | "func_arg1" | "func_arg2" | "func_arg3" :
                out.write("uint64_t %s = parseReg();\n" % field_name)

            case "imm_i32" | "imm_i64" :
                out.write("uint64_t %s = parseImmI();\n" % field_name)

            case "imm_f" :
                out.write("uint64_t %s = parseImmF();\n" % field_name)

            case "imm_d" :
                out.write("uint64_t %s = parseImmD();\n" % field_name)

            case "jump_offset" :
                out.write("ByteOffset jump_offset = 0;")
                out.write("auto dst_name = parseJumpDst();")

            case "func_id" :
                out.write("auto func_id = parseCallFuncId();")

            case "str_id" :
                out.write("auto str = parseString();\n")
                out.write("auto str_id = parseStrId(str);")

            case "class_id":
                out.write("auto class_id = parseDefinedClassId();")

            case "field_id":
                out.write("auto field_id = parseClassFieldId(class_id);")

            case _:
                raise RuntimeError("Unknown field in instruction %s: %s" % (instr.name, field_name))

    out.write("\n")
    out.write("instrs.push_back(std::make_unique<Instr<%s>>(" % instr.get_opcode_name())

    first = True
    for field_name in instr.fields.keys() :
        if not first :
            out.write(", ")
        first = False
        out.write("%s" % field_name)

    out.write("));\n\n")

    if instr.is_jump :
        out.write("curr_func_->jumps().emplace_back(static_cast<InterfaceJump*>(instrs.back().get()), curr_offset_, dst_name, lexer_.lineno());\n")

    out.write(
        "curr_offset_ += instrs.back()->getByteSize();\n"
        "}\n\n"
    )

def write_instr_map(out: TextIOWrapper, instrs: list) :
    out.write("static std::unordered_map<std::string, InstrOpcode> instrs = {")

    first = True
    for instr in instrs :
        if not first :
            out.write(", ")
        first = False

        out.write("{\"%s\", %s}" % (instr.name, instr.get_opcode_name()))

    out.write("};\n\n")

def write_parse_func_body(out: TextIOWrapper, instrs: list) :
    out.write(
        "int Assembler::parseFuncBody()\n"
        "{\n"
    )

    write_instr_map(out, instrs)

    out.write(
        "int lexing_status = Lexer::LEXING_ERROR_CODE;\n\n"

        "while ((lexing_status = lexer_.yylex()) == Lexer::LEXING_OK) {\n"
            "if (lexer_.currLexemType() == Lexer::LexemType::CLASS ||\
                 lexer_.currLexemType() == Lexer::LexemType::FUNC) {\n"
            "return lexing_status;\n"
            "}\n"
            "if (lexer_.currLexemType() == Lexer::LexemType::LABEL) {\n"
                "addLexedLabel();\n"
                "continue;\n"
            "}\n\n"

            "if (lexer_.currLexemType() == Lexer::LexemType::STRING) {\n"
                "parseString();\n"
                "continue;\n"
            "}\n\n"

            "assertParseError(lexer_.currLexemType() == Lexer::LexemType::IDENTIFIER);\n\n"

            "std::string id = lexer_.YYText();\n"
            "std::transform(id.begin(), id.end(), id.begin(), ::toupper);\n\n"

            "auto it = instrs.find(id);\n"
            "assertParseError(it != instrs.end());\n\n"

            "switch(it->second) {\n"
    )

    for instr in instrs :
        out.write("case %s:\n" % instr.get_opcode_name())
        out.write("parseInstr<%s>();\n" % instr.get_opcode_name())
        out.write("break;\n")

    out.write(
        "default: assert(0);\n"

                "}\n"
            "}\n\n"

            "return lexing_status;\n"
        "}\n"
    )

def write_file_close(out: TextIOWrapper) :
    out.write(
        "} // namespace shrimp::assembler\n\n"

        "#endif // PARSER_GEN_HPP\n\n"
    )

if __name__ == "__main__" :
    IN_NAME = sys.argv[1]
    OUT_NAME = sys.argv[2]

    instrs = load_instrs(IN_NAME)

    out = open(OUT_NAME, 'w')

    write_file_open(out)

    for instr in instrs :
        write_instr_parser(out, instr)

    write_parse_func_body(out, instrs)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
