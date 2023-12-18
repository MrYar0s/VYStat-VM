#ifndef FRONTEND_LANG2SHRIMP_HPP
#define FRONTEND_LANG2SHRIMP_HPP

#include <memory>
#include <shrimp/lexer.hpp>
#include <shrimp/parser.hpp>

#include <shrimp/common/types.hpp>

#include <shrimp/assembler/instr.gen.hpp>

#include <string>
#include <unordered_map>

#include <shrimp/shrimpfile.hpp>
#include "shrimp/frontend/astnode.hpp"

namespace shrimp {

class CompilerFuncInfo final {
public:
    CompilerFuncInfo(ByteOffset offset, std::string name, std::vector<std::string> args)
        : offset_(offset), name_(name), args_(args)
    {
    }

    auto getArgs() const noexcept
    {
        return args_;
    }
    auto &getInstrs() noexcept
    {
        return instrs_;
    }

    const auto &getName() const noexcept
    {
        return name_;
    }

    auto getOffset() const noexcept
    {
        return offset_;
    }

    auto &getRegMap() noexcept
    {
        return varIdent_to_reg_;
    }

private:
    // Function offset
    ByteOffset offset_ = 0;

    // Function name
    std::string name_ = "";

    // Function arguments aliases
    std::vector<std::string> args_ {};

    // Parsed instructions
    std::vector<std::unique_ptr<assembler::InterfaceInstr>> instrs_ {};

    std::unordered_map<std::string, std::pair<R8Id, ValueType>> varIdent_to_reg_;
};

class Compiler {
public:
    explicit Compiler(const std::string &input_file, const std::string &output_file)
        : input_file_ {input_file}, output_file_(output_file)
    {
    }

    void run();

    bool ast2file(std::unique_ptr<ASTNode> &&astRoot, std::string input_file, std::string output_file);
    void compileFunc(const std::unique_ptr<ASTNode> &func);
    void compileStatements(const ASTNode *node);

    void compileVarDecl(const std::unique_ptr<ASTNode> &instr);
    void compileRetStmt(const std::unique_ptr<ASTNode> &instr);
    void compileIfStmt(const std::unique_ptr<ASTNode> &instr);
    void compileExpr(const std::unique_ptr<ASTNode> &expr, const std::string &name);
    void compileLogic(const std::unique_ptr<ASTNode> &expr, const std::string &name);
    void compileArithm(const std::unique_ptr<ASTNode> &expr, const std::string &name);
    void compileArithmOperation(const std::unique_ptr<ASTNode> &expr, const std::string &source);
    void getFromExpr(const std::unique_ptr<ASTNode> &child);

    void write(shrimp::shrimpfile::File &out);
    void writeCode(shrimp::shrimpfile::File &out);
    void writeStrings(shrimp::shrimpfile::File &out);
    void writeFunctions(shrimp::shrimpfile::File &out);

private:
    std::string input_file_;
    std::string output_file_;

    ByteOffset curr_offset_ = 0;

    std::vector<std::unique_ptr<assembler::InterfaceInstr>> instrs_ {};

    std::unordered_map<std::string, FuncId> funcName_to_id_;
    std::unordered_map<std::string, StrId> strings_ {};

    std::vector<CompilerFuncInfo> funcs_ {};
    CompilerFuncInfo *curr_func_;

    Lexer lexer_;
    Parser parser_;
};

}  // namespace shrimp

#endif  // FRONTEND_LANG2SHRIMP_HPP