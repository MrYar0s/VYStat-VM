import os
import sys

from instr import*

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
    out.write("template<>\n")
    out.write("void Assembler::parseInstr<%s>() {\n" % instr.get_opcode_name())

    need_comma = False
    for field_name in instr.fields.keys() :
        if need_comma :
            out.write("parseComma();")

        need_comma = True

        match field_name :
            case "rd" | "rs" | "rs1" | "rs2" :
                out.write("uint64_t %s = parseReg();\n" % field_name)

            case "imm_i32" | "imm_i64" :
                out.write("uint64_t %s = parseImmI();\n" % field_name)

            case "imm_f" :
                out.write("uint64_t %s = parseImmF();\n" % field_name)

            case "imm_d" :
                out.write("uint64_t %s = parseImmD();\n" % field_name)

            case "jump_offset" :
                out.write("ByteOffset jump_offset = 0;")
                out.write("parseJumpDst();")

            case "intrinsic_code" :
                out.write("auto intrinsic_code_enum = parseIntrinsicName();")
                out.write("auto intrinsic_args = parseIntrinsicArgs(intrinsic_code_enum);")
                out.write("auto intrinsic_code = static_cast<uint8_t>(intrinsic_code_enum);")
                need_comma = False

            case "intrinsic_arg_0" | "intrinsic_arg_1" | "intrinsic_arg_2" | "intrinsic_arg_3" :
                out.write("R8Id %s = intrinsic_args[%c];" % (field_name, field_name[-1]))
                need_comma = False

            case _:
                raise RuntimeError("Unknown field in instruction %s: %s" % (instr.name, field_name))

    out.write("\n")
    out.write("instrs_.push_back(std::make_unique<Instr<%s>>(" % instr.get_opcode_name())

    first = True
    for field_name in instr.fields.keys() :
        if not first :
            out.write(", ")
        first = False
        out.write("%s" % field_name)

    out.write(
        "));\n\n"
        "curr_offset_ += instrs_.back()->getByteSize();\n"
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

def write_first_pass(out: TextIOWrapper, instrs: list) :
    out.write(
        "void Assembler::first_pass()\n"
        "{\n"
    )

    write_instr_map(out, instrs)

    out.write(
        "int lexing_status = Lexer::LEXING_ERROR_CODE;\n\n"

        "while ((lexing_status = lexer_.yylex()) == Lexer::LEXING_OK) {\n"
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
            "}\n"
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

    write_first_pass(out, instrs)

    write_file_close(out)

    out.close()

    os.system("clang-format -i %s" % OUT_NAME)
