#include <sys/types.h>
#include <memory>
#include <shrimp/lang2shrimp.hpp>
#include <shrimp/frontend/astnode.hpp>
#include <fstream>
#include <iostream>
#include <shrimp/shrimpfile.hpp>

namespace shrimp {

void Compiler::run()
{
    std::ifstream stream {program_file_};
    if (!stream) {
        std::cout << "Can't open " << program_file_ << std::endl;
        return;
    }

    std::string buffer {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char> {}};

    lexer_ = Lexer {buffer};
    lexer_.run();
    std::vector<Token> tokens = std::move(lexer_.getTokens());
    for (auto &&token : tokens) {
        std::cout << "\'" << token.value << "\'" << std::endl;
    }

    auto root = std::make_unique<ASTNode>(ASTNode::NodeKind::PROGRAM, "program");
    parser_.run(std::move(tokens), std::move(root));
    AstRet astRoot = parser_.getRoot();

    std::string output_file = "a.out";

    ast2file(std::move(astRoot.first), program_file_, output_file);
}

bool Compiler::ast2file(std::unique_ptr<ASTNode> &&astRoot, std::string input_file, std::string output_file)
{
    auto &&funcs = std::move(astRoot->GetChildrenNodes());
    for (auto const &func : funcs) {
        compileFunc(std::move(func));
    }

    shrimpfile::File of {input_file, output_file};

    write(of);

    return true;
}

void Compiler::write(shrimp::shrimpfile::File &out)
{
    writeCode(out);
    writeStrings(out);
    writeFunctions(out);
    out.fillHeaders();
    out.serialize();
}

void Compiler::writeCode(shrimp::shrimpfile::File &out)
{
    for (auto &&func : funcs_) {
        for (auto &&instr_ptr : func.getInstrs()) {
            const char *bin_code = reinterpret_cast<const char *>(instr_ptr->getBinCode());
            out.writeBytes(bin_code, instr_ptr->getByteSize());
        }
    }
}

void Compiler::writeStrings(shrimp::shrimpfile::File &out)
{
    for (auto [str, str_id] : strings_) {
        out.writeString(str, str_id);
    }
}

void Compiler::writeFunctions(shrimp::shrimpfile::File &out)
{
    for (auto &&func : funcs_) {
        shrimpfile::File::FileFunction file_func;
        file_func.name = func.getName();
        file_func.name_size = file_func.name.size();
        file_func.id = funcName_to_id_[file_func.name];
        file_func.func_start = func.getOffset();
        file_func.num_of_args = func.getArgs().size();
        file_func.num_of_vregs = 256;
        out.writeFunction(file_func);
    }
}

void Compiler::compileFunc(const std::unique_ptr<ASTNode> &ast)
{
    auto *func = reinterpret_cast<FunctionDecl *>(ast.get());

    std::vector<std::string> args {};

    funcs_.emplace_back(curr_offset_, func->GetName(), args);

    curr_func_ = &funcs_.back();

    auto &instrsuctions = func->GetChildrenNodes();

    for (const auto &instr : instrsuctions) {
        switch (instr->GetKind()) {
            case ASTNode::NodeKind::ASSIGN_EXPR:
                compileVarDecl(instr);
                break;
            case ASTNode::NodeKind::RETURN_STATEMENT:
                compileRetStmt(instr);
                break;
            default:
                std::abort();
                break;
        }
    }
}

void Compiler::compileRetStmt(const std::unique_ptr<ASTNode> &instr)
{
    auto *stmt = reinterpret_cast<RetStmt *>(instr.get());

    auto &instrs = curr_func_->getInstrs();

    const auto &childs = stmt->GetChildrenNodes();

    auto &expr = childs[0];

    std::string name = "<ret>";

    auto &node_name = expr->GetChildrenNodes()[0];

    if (node_name->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
        name = node_name->GetName();
    }

    varIdent_to_reg_.insert({name, varIdent_to_reg_.size()});
    compileExpr(expr, name);

    auto asm_lda = assembler::Instr<InstrOpcode::LDA>(varIdent_to_reg_[name]);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(asm_lda));
    curr_offset_ += asm_lda.getByteSize();

    auto asm_ret = assembler::Instr<InstrOpcode::RET>();
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::RET>>(asm_ret));
    curr_offset_ += asm_ret.getByteSize();
}

void Compiler::compileVarDecl(const std::unique_ptr<ASTNode> &instr)
{
    auto *var = reinterpret_cast<AssignExpr *>(instr.get());

    const auto &childs = var->GetChildrenNodes();

    auto &val = childs[0];

    if (val->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
        varIdent_to_reg_.insert({childs[0]->GetName(), varIdent_to_reg_.size()});
    }

    auto &expr = childs[1];
    if (expr->GetKind() == ASTNode::NodeKind::EXPR) {
        compileExpr(expr, val->GetName());
    }
}

void Compiler::compileArithm(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    auto &left = expr->GetChildrenNodes()[0];
    auto &right = expr->GetChildrenNodes()[1];

    std::string left_name = "<left>";
    std::string right_name = "<right>";

    if (left->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
        left_name = left->GetName();
    }
    if (right->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
        right_name = right->GetName();
    }
    varIdent_to_reg_.insert({left_name, varIdent_to_reg_.size()});
    compileExpr(left, left_name);
    auto asm_instr = assembler::Instr<InstrOpcode::LDA>(varIdent_to_reg_[left_name]);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(asm_instr));

    varIdent_to_reg_.insert({right_name, varIdent_to_reg_.size()});
    compileExpr(right, right_name);

    if (expr->GetName() == "*") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::MUL_I32>(varIdent_to_reg_[right_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MUL_I32>>(asm_op_instr));
    } else if (expr->GetName() == "/") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::DIV_I32>(varIdent_to_reg_[right_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::DIV_I32>>(asm_op_instr));
    } else if (expr->GetName() == "+") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ADD_I32>(varIdent_to_reg_[right_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ADD_I32>>(asm_op_instr));
    } else if (expr->GetName() == "-") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::SUB_I32>(varIdent_to_reg_[right_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::SUB_I32>>(asm_op_instr));
    }

    auto ret_instr = assembler::Instr<InstrOpcode::STA>(varIdent_to_reg_[name]);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(ret_instr));
}

void Compiler::compileExpr(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    if (expr->GetName() == "*" || expr->GetName() == "/" ||
        expr->GetName() == "+" || expr->GetName() == "-") {
        compileArithm(expr, name);
    } else {
        auto &childs = expr->GetChildrenNodes();

        for (auto &child : childs) {
            compileExpr(child, name);
        }
    }

    if (expr->GetKind() == ASTNode::NodeKind::NUMBER) {
        auto *number = reinterpret_cast<Number *>(expr.get());

        auto asm_instr =
            assembler::Instr<InstrOpcode::MOV_IMM_I32>(varIdent_to_reg_[name], number->getValue());

        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV_IMM_I32>>(asm_instr));
        curr_offset_ += asm_instr.getByteSize();
    }
}

}  // namespace shrimp