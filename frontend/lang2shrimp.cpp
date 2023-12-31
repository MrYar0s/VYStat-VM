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
    std::ifstream stream {input_file_};
    if (!stream) {
        std::cout << "Can't open " << input_file_ << std::endl;
        return;
    }

    std::string buffer {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char> {}};

    lexer_ = Lexer {buffer};
    lexer_.run();
    std::vector<Token> tokens = std::move(lexer_.getTokens());

    auto root = std::make_unique<ASTNode>(ASTNode::NodeKind::PROGRAM, "program");
    parser_.run(std::move(tokens), std::move(root));
    AstRet astRoot = parser_.getRoot();

    ast2file(std::move(astRoot.first), input_file_, output_file_);
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

void Compiler::compileStatements(const ASTNode *node)
{
    auto &instrsuctions = node->GetChildrenNodes();

    for (const auto &instr : instrsuctions) {
        switch (instr->GetKind()) {
            case ASTNode::NodeKind::ASSIGN_EXPR:
                compileVarDecl(instr);
                break;
            case ASTNode::NodeKind::RETURN_STATEMENT:
                compileRetStmt(instr);
                break;
            case ASTNode::NodeKind::IF_STATEMENT:
                compileIfStmt(instr);
                break;
            case ASTNode::NodeKind::FOR_STATEMENT:
                compileForStmt(instr);
                break;
            case ASTNode::NodeKind::FUNCTION_CALL:
                compileExpr(instr, "");
                break;
            default:
                std::abort();
                break;
        }
    }
}

void Compiler::compileFunc(const std::unique_ptr<ASTNode> &ast)
{
    auto *func = reinterpret_cast<FunctionDecl *>(ast.get());

    std::vector<std::string> compile_args;

    for (auto &arg : func->getArgs()) {
        compile_args.push_back(arg.first);
    }

    funcs_.emplace_back(curr_offset_, func->GetName(), compile_args);
    funcName_to_id_.insert({func->GetName(), funcs_.size()});

    curr_func_ = &funcs_.back();

    auto &regMap = curr_func_->getRegMap();

    R8Id pos = 255;
    for (auto &arg : func->getArgs()) {
        regMap.insert({arg.first, {pos, arg.second}});
    }

    compileStatements(func);
}

void Compiler::compileForStmt(const std::unique_ptr<ASTNode> &instr)
{
    auto &instrs = curr_func_->getInstrs();

    auto &init = instr->GetChildrenNodes()[0];
    auto &cond = instr->GetChildrenNodes()[1];
    auto &step = instr->GetChildrenNodes()[2];
    auto &stmts = instr->GetChildrenNodes()[3];

    compileVarDecl(init);

    std::string reg_for_init_stmt = "<for_init_stmt>";

    curr_func_->getRegMap().insert({reg_for_init_stmt, {curr_func_->getRegMap().size(), ValueType::INT}});
    uint64_t start = curr_offset_;
    compileExpr(cond, reg_for_init_stmt);

    auto if_stmt_instr = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[reg_for_init_stmt].first);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(if_stmt_instr));
    curr_offset_ += if_stmt_instr.getByteSize();

    auto mov_for_jmp = assembler::Instr<InstrOpcode::MOV_IMM_I32>(curr_func_->getRegMap()[reg_for_init_stmt].first, 0);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV_IMM_I32>>(mov_for_jmp));
    curr_offset_ += mov_for_jmp.getByteSize();

    auto jmp_instr_end = assembler::Instr<InstrOpcode::JUMP_EQ>(curr_func_->getRegMap()[reg_for_init_stmt].first, 0);
    uint64_t pos_end_jmp = instrs.size();
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::JUMP_EQ>>(jmp_instr_end));
    uint64_t pre_body = curr_offset_;
    curr_offset_ += jmp_instr_end.getByteSize();

    compileStatements(stmts.get());

    compileVarDecl(step);
    auto jmp_instr_start = assembler::Instr<InstrOpcode::JUMP>(0);
    uint64_t pos_start_jmp = instrs.size();
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::JUMP>>(jmp_instr_start));
    uint64_t end = curr_offset_;
    curr_offset_ += jmp_instr_start.getByteSize();

    auto parsed_jmp_start_instr = assembler::Instr<InstrOpcode::JUMP>(start - end);
    instrs[pos_start_jmp] = (std::make_unique<assembler::Instr<InstrOpcode::JUMP>>(parsed_jmp_start_instr));

    auto parsed_jmp_end_instr = assembler::Instr<InstrOpcode::JUMP_EQ>(
        curr_func_->getRegMap()[reg_for_init_stmt].first, end - pre_body + parsed_jmp_start_instr.getByteSize());
    instrs[pos_end_jmp] = (std::make_unique<assembler::Instr<InstrOpcode::JUMP_EQ>>(parsed_jmp_end_instr));
}

void Compiler::compileIfStmt(const std::unique_ptr<ASTNode> &instr)
{
    auto &instrs = curr_func_->getInstrs();

    auto &if_stmt = instr->GetChildrenNodes()[0];

    std::string reg_for_if_stmt = "<if_stmt>";

    curr_func_->getRegMap().insert({reg_for_if_stmt, {curr_func_->getRegMap().size(), ValueType::INT}});
    compileExpr(if_stmt, reg_for_if_stmt);

    auto if_stmt_instr = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[reg_for_if_stmt].first);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(if_stmt_instr));
    curr_offset_ += if_stmt_instr.getByteSize();

    auto mov_for_jmp = assembler::Instr<InstrOpcode::MOV_IMM_I32>(curr_func_->getRegMap()[reg_for_if_stmt].first, 0);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV_IMM_I32>>(mov_for_jmp));
    curr_offset_ += mov_for_jmp.getByteSize();

    auto jmp_instr = assembler::Instr<InstrOpcode::JUMP_EQ>(curr_func_->getRegMap()[reg_for_if_stmt].first, 0);
    uint64_t pos = instrs.size();
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::JUMP_EQ>>(jmp_instr));

    uint64_t start = curr_offset_;
    curr_offset_ += jmp_instr.getByteSize();

    compileStatements(instr->GetChildrenNodes()[1].get());
    uint64_t end = curr_offset_;

    auto parsed_jmp_instr =
        assembler::Instr<InstrOpcode::JUMP_EQ>(curr_func_->getRegMap()[reg_for_if_stmt].first, end - start);
    instrs[pos] = (std::make_unique<assembler::Instr<InstrOpcode::JUMP_EQ>>(parsed_jmp_instr));
}

void Compiler::compileRetStmt(const std::unique_ptr<ASTNode> &instr)
{
    auto *stmt = reinterpret_cast<RetStmt *>(instr.get());

    auto &instrs = curr_func_->getInstrs();

    const auto &childs = stmt->GetChildrenNodes();

    auto &expr = childs[0];

    std::string name = "<ret>";

    auto &node_name = expr->GetChildrenNodes()[0];

    if (node_name->GetKind() == ASTNode::NodeKind::IDENTIFIER && expr->GetName() != "<" && expr->GetName() != ">" &&
        expr->GetName() != "==") {
        name = node_name->GetName();
    }

    curr_func_->getRegMap().insert({name, {curr_func_->getRegMap().size(), ValueType::INT}});
    compileExpr(expr, name);

    auto asm_lda = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[name].first);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(asm_lda));
    curr_offset_ += asm_lda.getByteSize();

    auto asm_ret = assembler::Instr<InstrOpcode::RET>();
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::RET>>(asm_ret));
    curr_offset_ += asm_ret.getByteSize();
}

void Compiler::compileArrInit(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    auto *arr = reinterpret_cast<Array *>(expr.get());
    auto type = arr->getType();

    if (type == ValueType::INT) {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ARR_NEW_I32>(curr_func_->getRegMap()[expr->GetName()].first,
                                                                       curr_func_->getRegMap()[name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ARR_NEW_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (type == ValueType::FLOAT) {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ARR_NEW_F>(curr_func_->getRegMap()[expr->GetName()].first,
                                                                     curr_func_->getRegMap()[name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ARR_NEW_F>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    }
}

void Compiler::compileArrStore(const std::unique_ptr<ASTNode> &expr, const std::string &tmp_for_idx,
                               const std::string &need_to_assign)
{
    auto &instrs = curr_func_->getInstrs();

    auto *arr = reinterpret_cast<Array *>(expr.get());
    auto type = arr->getType();

    auto load_instr = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[need_to_assign].first);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(load_instr));
    curr_offset_ += load_instr.getByteSize();
    if (type == ValueType::INT) {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ARR_STA_I32>(curr_func_->getRegMap()[expr->GetName()].first,
                                                                       curr_func_->getRegMap()[tmp_for_idx].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ARR_STA_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (type == ValueType::FLOAT) {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ARR_STA_F>(curr_func_->getRegMap()[expr->GetName()].first,
                                                                     curr_func_->getRegMap()[tmp_for_idx].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ARR_STA_F>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    }
}

void Compiler::compileArrLoad(const std::unique_ptr<ASTNode> &expr, const std::string &tmp_for_idx,
                              const std::string &assign_to)
{
    auto &instrs = curr_func_->getInstrs();

    auto *arr = reinterpret_cast<Array *>(expr.get());
    auto type = arr->getType();

    if (type == ValueType::INT) {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ARR_LDA_I32>(curr_func_->getRegMap()[expr->GetName()].first,
                                                                       curr_func_->getRegMap()[tmp_for_idx].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ARR_LDA_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (type == ValueType::FLOAT) {
        auto asm_op_instr = assembler::Instr<InstrOpcode::ARR_LDA_F>(curr_func_->getRegMap()[expr->GetName()].first,
                                                                     curr_func_->getRegMap()[tmp_for_idx].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ARR_LDA_F>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    }

    auto load_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[assign_to].first);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(load_instr));
    curr_offset_ += load_instr.getByteSize();
}

void Compiler::compileVarDecl(const std::unique_ptr<ASTNode> &instr)
{
    auto *var = reinterpret_cast<AssignExpr *>(instr.get());

    const auto &childs = var->GetChildrenNodes();

    auto &val = childs[0];

    if (val->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
        auto ident_node = reinterpret_cast<Identifier *>(val.get());
        curr_func_->getRegMap().insert({val->GetName(), {curr_func_->getRegMap().size(), ident_node->getType()}});
        auto &expr = childs[1];
        if (expr->GetKind() == ASTNode::NodeKind::EXPR) {
            compileExpr(expr, val->GetName());
        }
    }

    if (childs.size() == 1) {  // Array creation case
        if (val->GetKind() == ASTNode::NodeKind::ARRAY) {
            auto *arr = reinterpret_cast<Array *>(val.get());
            auto &child = val->GetChildrenNodes()[0];
            if (child->GetKind() == ASTNode::NodeKind::NUMBER) {
                auto *number = reinterpret_cast<Number *>(child.get());
                curr_func_->getRegMap().insert({arr->GetName(), {curr_func_->getRegMap().size(), arr->getType()}});
                curr_func_->getRegMap().insert(
                    {number->getTmpName(), {curr_func_->getRegMap().size(), number->getType()}});
                compileExpr(child, number->getTmpName());
                compileArrInit(val, number->getTmpName());
            }
        }
    } else {
        auto &expr = childs[1];
        if (val->GetKind() == ASTNode::NodeKind::ARRAY) {
            auto *arr = reinterpret_cast<Array *>(val.get());
            auto &child = val->GetChildrenNodes()[0];
            std::string tmp_for_idx = "<idx_for_store_arr>";
            curr_func_->getRegMap().insert({tmp_for_idx, {curr_func_->getRegMap().size(), ValueType::INT}});
            compileExpr(child, tmp_for_idx);
            std::string need_to_assign = "<assign_val_arr>";
            curr_func_->getRegMap().insert({need_to_assign, {curr_func_->getRegMap().size(), arr->getType()}});
            compileExpr(expr, need_to_assign);
            compileArrStore(val, tmp_for_idx, need_to_assign);
        }
    }
}

void Compiler::compileArithmOperation(const std::unique_ptr<ASTNode> &expr, const std::string &source)
{
    auto &instrs = curr_func_->getInstrs();

    auto pair = curr_func_->getRegMap()[source];

    if (pair.second == ValueType::INT) {
        if (expr->GetName() == "*") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::MUL_I32>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MUL_I32>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        } else if (expr->GetName() == "/") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::DIV_I32>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::DIV_I32>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        } else if (expr->GetName() == "+") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::ADD_I32>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ADD_I32>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        } else if (expr->GetName() == "-") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::SUB_I32>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::SUB_I32>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        }
    } else if (pair.second == ValueType::FLOAT) {
        if (expr->GetName() == "*") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::MUL_F>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MUL_F>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        } else if (expr->GetName() == "/") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::DIV_F>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::DIV_F>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        } else if (expr->GetName() == "+") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::ADD_F>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::ADD_F>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        } else if (expr->GetName() == "-") {
            auto asm_op_instr = assembler::Instr<InstrOpcode::SUB_F>(pair.first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::SUB_F>>(asm_op_instr));
            curr_offset_ += asm_op_instr.getByteSize();
        }
    }
}

void Compiler::compileLogic(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    for (auto &child : expr->GetChildrenNodes()) {
        if (child->GetKind() == ASTNode::NodeKind::NUMBER) {
            auto *child_number = reinterpret_cast<Number *>(child.get());
            auto child_name = child_number->getTmpName();
            curr_func_->getRegMap().insert({child_name, {curr_func_->getRegMap().size(), child_number->getType()}});
            child->setName(child_name);
            compileExpr(child, child_name);
        } else if (child->GetKind() == ASTNode::NodeKind::EXPR) {
            if (child->GetChildrenNodes().size() == 1) {
                auto *child_of_child = child->GetChildrenNodes()[0].get();
                while (child_of_child->GetKind() == ASTNode::NodeKind::EXPR && child->GetChildrenNodes().size() == 1) {
                    child_of_child = child_of_child->GetChildrenNodes()[0].get();
                }
                if (child_of_child->GetKind() == ASTNode::NodeKind::NUMBER) {
                    auto *child_number = reinterpret_cast<Number *>(child_of_child);
                    auto child_name = child_number->getTmpName();
                    curr_func_->getRegMap().insert(
                        {child_name, {curr_func_->getRegMap().size(), child_number->getType()}});
                    child->setName(child_name);
                    compileExpr(child, child_name);
                } else if (child_of_child->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
                    auto child_name = child_of_child->GetName();
                    child->setName(child_name);
                    compileExpr(child, child_name);
                }
            }
        } else if (child->GetKind() == ASTNode::NodeKind::ARRAY) {
            std::string tmp_reg_for_arr = "<tmp_reg_arr>";
            auto *arr = reinterpret_cast<Array *>(child.get());
            curr_func_->getRegMap().insert({tmp_reg_for_arr, {curr_func_->getRegMap().size(), arr->getType()}});
            compileExpr(child, tmp_reg_for_arr);
            child->setName(tmp_reg_for_arr);
        }
    }

    std::string left_name = expr->GetChildrenNodes()[0]->GetName();
    std::string right_name = expr->GetChildrenNodes()[1]->GetName();

    auto asm_instr = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[left_name].first);
    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(asm_instr));
    curr_offset_ += asm_instr.getByteSize();

    if (expr->GetName() == ">") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::CMP_GG_I32>(curr_func_->getRegMap()[right_name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CMP_GG_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (expr->GetName() == "<") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::CMP_LL_I32>(curr_func_->getRegMap()[right_name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CMP_LL_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    } else if (expr->GetName() == "==") {
        auto asm_op_instr = assembler::Instr<InstrOpcode::CMP_EQ_I32>(curr_func_->getRegMap()[right_name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CMP_EQ_I32>>(asm_op_instr));
        curr_offset_ += asm_op_instr.getByteSize();
    }

    if (name == "<ret>" || curr_func_->getRegMap().find(name) != curr_func_->getRegMap().end()) {
        auto ret_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(ret_instr));
        curr_offset_ += ret_instr.getByteSize();
    }
}

void Compiler::compileArithm(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    for (auto &child : expr->GetChildrenNodes()) {
        if (child->GetName() == "*" || child->GetName() == "/" || child->GetName() == "+" || child->GetName() == "-") {
            compileExpr(child, "");
        } else if (child->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
            auto child_name = child->GetName();
            child->setName(child_name);
            compileExpr(child, child_name);
        } else if (child->GetKind() == ASTNode::NodeKind::NUMBER) {
            auto *child_number = reinterpret_cast<Number *>(child.get());
            auto child_name = child_number->getTmpName();
            curr_func_->getRegMap().insert({child_name, {curr_func_->getRegMap().size(), child_number->getType()}});
            child->setName(child_name);
            compileExpr(child, child_name);
        } else if (child->GetKind() == ASTNode::NodeKind::EXPR) {
            if (child->GetChildrenNodes().size() == 1) {
                auto *child_of_child = child->GetChildrenNodes()[0].get();
                while (child_of_child->GetKind() == ASTNode::NodeKind::EXPR && child->GetChildrenNodes().size() == 1) {
                    child_of_child = child_of_child->GetChildrenNodes()[0].get();
                }
                if (child_of_child->GetKind() == ASTNode::NodeKind::NUMBER) {
                    auto *child_number = reinterpret_cast<Number *>(child_of_child);
                    auto child_name = child_number->getTmpName();
                    curr_func_->getRegMap().insert(
                        {child_name, {curr_func_->getRegMap().size(), child_number->getType()}});
                    child->setName(child_name);
                    compileExpr(child, child_name);
                } else if (child_of_child->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
                    auto child_name = child_of_child->GetName();
                    child->setName(child_name);
                    compileExpr(child, child_name);
                }
            }
        }
    }

    std::string left_name = expr->GetChildrenNodes()[0]->GetName();
    std::string right_name = expr->GetChildrenNodes()[1]->GetName();

    if (right_name == "*" || right_name == "/" || right_name == "+" || right_name == "-") {
        std::string tmp_swap_reg = "<tmp_swap>";
        curr_func_->getRegMap().insert({tmp_swap_reg, {curr_func_->getRegMap().size(), ValueType::INT}});
        auto tmp_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[tmp_swap_reg].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(tmp_instr));
        curr_offset_ += tmp_instr.getByteSize();

        auto swap_instr_1 = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[left_name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(swap_instr_1));
        curr_offset_ += swap_instr_1.getByteSize();

        auto swap_instr_2 = assembler::Instr<InstrOpcode::MOV>(curr_func_->getRegMap()[tmp_swap_reg].first,
                                                               curr_func_->getRegMap()[left_name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV>>(swap_instr_2));
        curr_offset_ += swap_instr_2.getByteSize();
    }

    if (right_name != "*" && right_name != "/" && right_name != "+" && right_name != "-" && left_name != "*" &&
        left_name != "/" && left_name != "+" && left_name != "-") {
        auto asm_instr = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[left_name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(asm_instr));
        curr_offset_ += asm_instr.getByteSize();
    }

    compileArithmOperation(expr, right_name);

    if (name == "<ret>" || curr_func_->getRegMap().find(name) != curr_func_->getRegMap().end()) {
        auto ret_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(ret_instr));
        curr_offset_ += ret_instr.getByteSize();
    }
}

void Compiler::compileExpr(const std::unique_ptr<ASTNode> &expr, const std::string &name)
{
    auto &instrs = curr_func_->getInstrs();

    if (expr->GetName() == "*" || expr->GetName() == "/" || expr->GetName() == "+" || expr->GetName() == "-") {
        compileArithm(expr, name);
    } else if (expr->GetName() == "<" || expr->GetName() == ">" || expr->GetName() == "==") {
        compileLogic(expr, name);
    } else {
        auto &childs = expr->GetChildrenNodes();

        for (auto &child : childs) {
            compileExpr(child, name);
        }
    }

    if (expr->GetKind() == ASTNode::NodeKind::NUMBER) {
        auto *number = reinterpret_cast<Number *>(expr.get());

        if (number->getType() == ValueType::INT) {
            auto asm_instr =
                assembler::Instr<InstrOpcode::MOV_IMM_I32>(curr_func_->getRegMap()[name].first, number->getValue());
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV_IMM_I32>>(asm_instr));
            curr_offset_ += asm_instr.getByteSize();
        } else if (number->getType() == ValueType::FLOAT) {
            auto asm_instr =
                assembler::Instr<InstrOpcode::MOV_IMM_F>(curr_func_->getRegMap()[name].first, number->getValue());
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV_IMM_F>>(asm_instr));
            curr_offset_ += asm_instr.getByteSize();
        }
    }

    if (expr->GetKind() == ASTNode::NodeKind::STRING) {
        auto *str = reinterpret_cast<String *>(expr.get());

        strings_.insert({str->getValue(), strings_.size()});

        auto lda_instr = assembler::Instr<InstrOpcode::LDA_STR>(strings_[str->getValue()]);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA_STR>>(lda_instr));
        curr_offset_ += lda_instr.getByteSize();

        auto asm_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(asm_instr));
        curr_offset_ += asm_instr.getByteSize();
    }

    if (expr->GetKind() == ASTNode::NodeKind::IDENTIFIER) {
        auto *ident = reinterpret_cast<Identifier *>(expr.get());

        auto asm_instr = assembler::Instr<InstrOpcode::MOV>(curr_func_->getRegMap()[ident->GetName()].first,
                                                            curr_func_->getRegMap()[name].first);
        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::MOV>>(asm_instr));
        curr_offset_ += asm_instr.getByteSize();
    }

    if (expr->GetKind() == ASTNode::NodeKind::ARRAY) {
        auto &child = expr->GetChildrenNodes()[0];
        std::string index_to_load = "<idx_for_load_arr>";
        curr_func_->getRegMap().insert({index_to_load, {curr_func_->getRegMap().size(), ValueType::INT}});
        compileExpr(child, index_to_load);
        compileArrLoad(expr, index_to_load, name);
    }

    if (expr->GetKind() == ASTNode::NodeKind::FUNCTION_CALL) {
        auto *funcCall = reinterpret_cast<FunctionCall *>(expr.get());

        auto &func_name = funcCall->GetName();
        auto &args = funcCall->getArgs();
        auto type = funcCall->getIntrinsicType();

        if (type == ASTNode::IntrinsicType::NONE) {
            switch (funcCall->getArgs().size()) {
                case 0: {
                    auto asm_instr = assembler::Instr<InstrOpcode::CALL_0ARG>(funcName_to_id_[func_name]);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_0ARG>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                case 1: {
                    auto asm_instr = assembler::Instr<InstrOpcode::CALL_1ARG>(funcName_to_id_[func_name],
                                                                              curr_func_->getRegMap()[args[0]].first);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_1ARG>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                case 2: {
                    auto asm_instr = assembler::Instr<InstrOpcode::CALL_2ARG>(funcName_to_id_[func_name],
                                                                              curr_func_->getRegMap()[args[0]].first,
                                                                              curr_func_->getRegMap()[args[1]].first);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_2ARG>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                case 3: {
                    auto asm_instr = assembler::Instr<InstrOpcode::CALL_3ARG>(
                        funcName_to_id_[func_name], curr_func_->getRegMap()[args[0]].first,
                        curr_func_->getRegMap()[args[1]].first, curr_func_->getRegMap()[args[2]].first);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_3ARG>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                case 4: {
                    auto asm_instr = assembler::Instr<InstrOpcode::CALL_4ARG>(
                        funcName_to_id_[func_name], curr_func_->getRegMap()[args[0]].first,
                        curr_func_->getRegMap()[args[1]].first, curr_func_->getRegMap()[args[2]].first,
                        curr_func_->getRegMap()[args[3]].first);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::CALL_4ARG>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                default:
                    std::abort();
                    break;
            }
        } else {
            auto &args = funcCall->getArgs();
            switch (type) {
                case ASTNode::IntrinsicType::SCAN: {
                    if (curr_func_->getRegMap()[name].second == ValueType::INT) {
                        auto asm_instr = assembler::Instr<InstrOpcode::INTRINSIC>(
                            static_cast<uint8_t>(IntrinsicCode::SCAN_I32), 0, 0, 0, 0);
                        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                        curr_offset_ += asm_instr.getByteSize();
                    } else if (curr_func_->getRegMap()[name].second == ValueType::FLOAT) {
                        auto asm_instr = assembler::Instr<InstrOpcode::INTRINSIC>(
                            static_cast<uint8_t>(IntrinsicCode::SCAN_F), 0, 0, 0, 0);
                        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                        curr_offset_ += asm_instr.getByteSize();
                    }
                    break;
                }
                case ASTNode::IntrinsicType::SQRT: {
                    auto asm_instr = assembler::Instr<InstrOpcode::INTRINSIC>(
                        static_cast<uint8_t>(IntrinsicCode::SQRT), curr_func_->getRegMap()[args[0]].first, 0, 0, 0);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                case ASTNode::IntrinsicType::PRINT: {
                    if (curr_func_->getRegMap()[args[0]].second == ValueType::INT) {
                        auto asm_instr =
                            assembler::Instr<InstrOpcode::INTRINSIC>(static_cast<uint8_t>(IntrinsicCode::PRINT_I32),
                                                                     curr_func_->getRegMap()[args[0]].first, 0, 0, 0);
                        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                        curr_offset_ += asm_instr.getByteSize();
                    } else if (curr_func_->getRegMap()[args[0]].second == ValueType::FLOAT) {
                        auto asm_instr =
                            assembler::Instr<InstrOpcode::INTRINSIC>(static_cast<uint8_t>(IntrinsicCode::PRINT_F),
                                                                     curr_func_->getRegMap()[args[0]].first, 0, 0, 0);
                        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                        curr_offset_ += asm_instr.getByteSize();
                    } else if (curr_func_->getRegMap()[args[0]].second == ValueType::STRING) {
                        auto asm_instr =
                            assembler::Instr<InstrOpcode::INTRINSIC>(static_cast<uint8_t>(IntrinsicCode::PRINT_STR),
                                                                     curr_func_->getRegMap()[args[0]].first, 0, 0, 0);
                        instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                        curr_offset_ += asm_instr.getByteSize();
                    }
                    break;
                }
                case ASTNode::IntrinsicType::CONCAT: {
                    auto asm_instr = assembler::Instr<InstrOpcode::INTRINSIC>(
                        static_cast<uint8_t>(IntrinsicCode::CONCAT), curr_func_->getRegMap()[args[0]].first,
                        curr_func_->getRegMap()[args[1]].first, 0, 0);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                case ASTNode::IntrinsicType::SUBSTR: {
                    auto sta_instr = assembler::Instr<InstrOpcode::LDA>(curr_func_->getRegMap()[args[0]].first);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::LDA>>(sta_instr));
                    curr_offset_ += sta_instr.getByteSize();

                    auto asm_instr = assembler::Instr<InstrOpcode::INTRINSIC>(
                        static_cast<uint8_t>(IntrinsicCode::SUBSTR), curr_func_->getRegMap()[args[1]].first,
                        curr_func_->getRegMap()[args[2]].first, 0, 0);
                    instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::INTRINSIC>>(asm_instr));
                    curr_offset_ += asm_instr.getByteSize();
                    break;
                }
                default:
                    std::abort();
                    break;
            }
        }
        if (type != ASTNode::IntrinsicType::PRINT) {
            auto ret_instr = assembler::Instr<InstrOpcode::STA>(curr_func_->getRegMap()[name].first);
            instrs.emplace_back(std::make_unique<assembler::Instr<InstrOpcode::STA>>(ret_instr));
            curr_offset_ += ret_instr.getByteSize();
        }
    }
}

}  // namespace shrimp