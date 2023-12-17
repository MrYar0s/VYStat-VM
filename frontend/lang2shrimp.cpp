#include <sys/types.h>
#include <cstdint>
#include <memory>
#include <shrimp/lang2shrimp.hpp>
#include <shrimp/frontend/astnode.hpp>
#include <fstream>
#include <iostream>
#include <shrimp/shrimpfile.hpp>
#include "shrimp/common/types.hpp"

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

    funcs_.emplace_back(curr_offset_, func->GetName(), func->getArgs());
    funcName_to_id_.insert({func->GetName(), funcs_.size()});

    curr_func_ = &funcs_.back();

    auto &regMap = curr_func_->getRegMap();

    R8Id pos = 255;
    for (auto &arg : func->getArgs()) {
        regMap.insert({arg, pos});
    }

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

    curr_func_->getRegMap().insert({name, curr_func_->getRegMap().size()});
    compileExpr(expr, name);

    auto asm_lda = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[name]);
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
        curr_func_->getRegMap().insert({childs[0]->GetName(), curr_func_->getRegMap().size()});
    }

    auto &expr = childs[1];
    if (expr->GetKind() == ASTNode::NodeKind::EXPR) {
        compileExpr(expr, val->GetName());
    }
}

void Compiler::compileArithmOperation(const std::unique_ptr<ASTNode> &expr, const std::string &source)
{
    auto &instrs = curr_func_->getInstrs();

    if (expr->GetName() == "*") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::MUL_I32>(curr_func_->getRegMap()[source]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MUL_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (expr->GetName() == "/") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::DIV_I32>(curr_func_->getRegMap()[source]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::DIV_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (expr->GetName() == "+") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ADD_I32>(curr_func_->getRegMap()[source]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ADD_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (expr->GetName() == "-") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::SUB_I32>(curr_func_->getRegMap()[source]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::SUB_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    }
}

void Compiler::compileArithm(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    std::cout << expr->GetName() << std::endl;
    std::cout << expr->GetChildrenNodes().size() << std::endl;

    auto &left = expr->GetChildrenNodes()[0];
    auto &right = expr->GetChildrenNodes()[1];

    std::cout << "Left name: " << left->GetName() << std::endl;
    std::cout << "Right name: " << right->GetName() << std::endl;

    for (auto &child : expr->GetChildrenNodes()) {
        if (child->GetName() == "*" || child->GetName() == "/" || child->GetName() == "+" || child->GetName() == "-") {
            compileExpr(child, "");
        } else if (child->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
            auto child_name = child->GetName();
            std::cout << "Ident name : " << child_name << std::endl;
            child->setName(child_name);
            compileExpr(child, child_name);
        } else if (child->GetKind() == ASTNode::NodeKind::NUMBER) {
            auto *child_number = reinterpret_cast<Number *>(child.get());
            auto child_name = child_number->getTmpName();
            curr_func_->getRegMap().insert({child_name, curr_func_->getRegMap().size()});
            std::cout << "Number name : " << child_name << std::endl;
            child->setName(child_name);
            compileExpr(child, child_name);
        } else if (child->GetKind() == ASTNode::NodeKind::EXPR) {
            if (child->GetChildrenNodes().size() == 1) {
                auto *child_of_child = child->GetChildrenNodes()[0].get();
                while(child_of_child->GetKind() == ASTNode::NodeKind::EXPR && child->GetChildrenNodes().size() == 1) {
                    child_of_child = child_of_child->GetChildrenNodes()[0].get();
                }
                if (child_of_child->GetKind() == ASTNode::NodeKind::NUMBER) {
                    auto *child_number = reinterpret_cast<Number *>(child_of_child);
                    auto child_name = child_number->getTmpName();
                    curr_func_->getRegMap().insert({child_name, curr_func_->getRegMap().size()});
                    std::cout << "Number name : " << child_name << std::endl;
                    child->setName(child_name);
                    compileExpr(child, child_name);
                } else if (child_of_child->GetKind() == ASTNode::NodeKind::IDENTIFIER){
                    auto child_name = child_of_child->GetName();
                    std::cout << "Ident name : " << child_name << std::endl;
                    child->setName(child_name);
                    compileExpr(child, child_name);
                }
            }
        }
    }

    std::string left_name = expr->GetChildrenNodes()[0]->GetName();
    std::string right_name = expr->GetChildrenNodes()[1]->GetName();

    std::cout << left_name << std::endl;
    std::cout << right_name << std::endl;

    if (right_name == "*" || right_name == "/" || right_name == "+" || right_name == "-") {
        std::string tmp_swap_reg = "<tmp_swap>";
        curr_func_->getRegMap().insert({tmp_swap_reg, curr_func_->getRegMap().size()});
        auto tmp_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[tmp_swap_reg]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(tmp_instr));
        curr_offset_ += tmp_instr.getByteSize();

        auto swap_instr_1 = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[left_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(swap_instr_1));
        curr_offset_ += swap_instr_1.getByteSize();

        auto swap_instr_2 = assembler::Instr<InstrOpcode::MOV>(curr_func_->getRegMap()[tmp_swap_reg], curr_func_->getRegMap()[left_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV>>(swap_instr_2));
        curr_offset_ += swap_instr_2.getByteSize();
    }

    if (right_name != "*" && right_name != "/" && right_name != "+" && right_name != "-" &&
        left_name != "*" && left_name != "/" && left_name != "+" && left_name != "-") {
        auto asm_instr= assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[left_name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(asm_instr));
        curr_offset_ += asm_instr.getByteSize();
    }

    compileArithmOperation(expr, right_name);

    if (name == "<ret>" || curr_func_->getRegMap().find(name) != curr_func_->getRegMap().end())
    {
        auto ret_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(ret_instr));
        curr_offset_ += ret_instr.getByteSize();
    }
}

void Compiler::compileExpr(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    if (expr->GetName() == "*" || expr->GetName() == "/" || expr->GetName() == "+" || expr->GetName() == "-") {
        compileArithm(expr, name);
    } else {
        auto &childs = expr->GetChildrenNodes();

        for (auto &child : childs) {
            compileExpr(child, name);
        }
    }

    if (expr->GetKind() == ASTNode::NodeKind::NUMBER) {
        auto *number = reinterpret_cast<Number *>(expr.get());

        auto asm_instr = assembler::Instr<InstrOpcode::MOV_IMM_I32>(curr_func_->getRegMap()[name], number->getValue());

        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV_IMM_I32>>(asm_instr));
        curr_offset_ += asm_instr.getByteSize();
    }

    if (expr->GetKind() == ASTNode::NodeKind::FUNCTION_CALL) {
        auto *funcCall = reinterpret_cast<FunctionCall *>(expr.get());

        auto &func_name = funcCall->GetName();
        auto &args = funcCall->getArgs();

        switch (funcCall->getArgs().size()) {
            case 0: {
                auto asm_instr = assembler::Instr<InstrOpcode::CALL_0ARG>(funcName_to_id_[func_name]);
                instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_0ARG>>(asm_instr));
                curr_offset_ += asm_instr.getByteSize();
                break;
            }
            case 1: {
                auto asm_instr =
                    assembler::Instr<InstrOpcode::CALL_1ARG>(funcName_to_id_[func_name], curr_func_->getRegMap()[args[0]]);
                instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_1ARG>>(asm_instr));
                curr_offset_ += asm_instr.getByteSize();
                break;
            }
            case 2: {
                auto asm_instr = assembler::Instr<InstrOpcode::CALL_2ARG>(
                    funcName_to_id_[func_name], curr_func_->getRegMap()[args[0]], curr_func_->getRegMap()[args[1]]);
                instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_2ARG>>(asm_instr));
                curr_offset_ += asm_instr.getByteSize();
                break;
            }
            case 3: {
                auto asm_instr = assembler::Instr<InstrOpcode::CALL_3ARG>(
                    funcName_to_id_[func_name], curr_func_->getRegMap()[args[0]], curr_func_->getRegMap()[args[1]],
                    curr_func_->getRegMap()[args[2]]);
                instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_3ARG>>(asm_instr));
                curr_offset_ += asm_instr.getByteSize();
                break;
            }
            case 4: {
                auto asm_instr = assembler::Instr<InstrOpcode::CALL_4ARG>(
                    funcName_to_id_[func_name], curr_func_->getRegMap()[args[0]], curr_func_->getRegMap()[args[1]],
                    curr_func_->getRegMap()[args[2]], curr_func_->getRegMap()[args[3]]);
                instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_4ARG>>(asm_instr));
                curr_offset_ += asm_instr.getByteSize();
                break;
            }
            default:
                std::abort();
                break;
        }
        auto ret_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[name]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(ret_instr));
        curr_offset_ += ret_instr.getByteSize();
    }
}

}  // namespace shrimp